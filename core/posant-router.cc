// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only

#include "posant-router.h"

#include "swarm-selection.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace posant::core {

using swarm::core::kInvalidAddress;

Router::Router(NodeAddress self, const Config& config, swarm::core::IRng& rng,
               const swarm::core::INeighborProvider& neighbours, Locate locate)
    : self_(self), config_(config), rng_(rng), neighbours_(neighbours), locate_(std::move(locate)),
      pheromone_(config.pheromoneCapacity), dataSeen_(config.dataHistoryCapacity) {}

std::vector<NodeAddress> Router::neighbours() const {
    std::vector<NodeAddress> n = neighbours_.neighbors();
    std::sort(n.begin(), n.end());
    n.erase(std::unique(n.begin(), n.end()), n.end());
    return n;
}

std::optional<Zone> Router::zoneFor(NodeAddress neighbour, NodeAddress dest) const {
    const std::optional<Vec3> me = locate_(self_);
    const std::optional<Vec3> n = locate_(neighbour);
    const std::optional<Vec3> d = locate_(dest);
    if (!me || !n || !d) {
        return std::nullopt;
    }
    return zoneOf(*me, *n, *d);
}

void Router::initialise(NodeAddress dest) {
    // [K07t] §5.2: every outgoing link gets nu_i of its zone. A neighbour
    // with no known position gets zone 2's value (repo choice).
    for (NodeAddress n : neighbours()) {
        const Zone z = zoneFor(n, dest).value_or(Zone::Two);
        pheromone_.set(dest, n, config_.initialPheromone[index(z)]);
    }
}

bool Router::hasTrail(NodeAddress dest) const {
    for (NodeAddress n : neighbours()) {
        if (pheromone_.get(dest, n) > 0.0) {
            return true;
        }
    }
    return false;
}

NodeAddress Router::choose(NodeAddress dest, std::optional<Zone> onlyZone) {
    if (pheromone_.row(dest).empty()) {
        initialise(dest);
    }
    // [K07t] eq. 2: p_i = phi_i / sum_j phi_j over the neighbours, the
    // previous hop included.
    std::vector<swarm::core::WeightedHop> candidates;
    for (NodeAddress n : neighbours()) {
        const double phi = pheromone_.get(dest, n);
        if (phi <= 0.0) {
            continue;
        }
        if (onlyZone && zoneFor(n, dest).value_or(Zone::Two) != *onlyZone) {
            continue;
        }
        candidates.push_back({n, phi});
    }
    return swarm::core::rouletteSelect(candidates, rng_);
}

void Router::touch(NodeAddress dest, Time now) {
    lastUse_[dest] = now;
}

void Router::rememberBackRoute(NodeAddress source, std::uint32_t seq, BackRoute route) {
    const auto key = std::make_pair(source, seq);
    if (backRouting_.contains(key)) {
        return;
    }
    // The order queue holds every live key (and the keys of consumed routes
    // until they reach its front), so capping it caps both (org rule 7).
    while (backRoutingOrder_.size() >= config_.backRoutingCapacity && !backRoutingOrder_.empty()) {
        backRouting_.erase(backRoutingOrder_.front());
        backRoutingOrder_.pop_front();
    }
    backRouting_.emplace(key, route);
    backRoutingOrder_.push_back(key);
}

bool Router::sendData(NodeAddress dest, Time now) {
    if (dest == self_) {
        return false;
    }
    SourceState& s = sources_[dest];
    if (s.queued >= config_.sourceQueue) {
        return false;
    }
    ++s.queued;
    touch(dest, now);
    return true;
}

std::vector<Action> Router::receive(NodeAddress from, const Header& h, Time now) {
    std::vector<Action> out;
    const NodeAddress dest = h.destination;
    switch (h.type) {
    case PacketType::Forward: {
        touch(dest, now);
        if (dest == self_) {
            // [K07t] §5.3: the forward ant ends here; a backward ant with the
            // same sequence number goes back the way it came.
            Header back = h;
            back.type = PacketType::Backward;
            back.hops = 1;
            out.push_back({Action::Kind::Send, from, back});
            return out;
        }
        if (backRouting_.contains({h.source, h.seq})) {
            return out;  // a repeated forward ant is destroyed
        }
        rememberBackRoute(h.source, h.seq, {from, dest});
        if (h.hops >= config_.maxHops) {
            return out;
        }
        const NodeAddress next = choose(dest, std::nullopt);
        if (next != kInvalidAddress) {
            Header fwd = h;
            fwd.hops = static_cast<std::uint16_t>(h.hops + 1);
            out.push_back({Action::Kind::Send, next, fwd});
        }
        return out;
    }
    case PacketType::Backward: {
        touch(dest, now);
        if (pheromone_.row(dest).empty()) {
            initialise(dest);
        }
        // [K07t] eq. 3: phi_AB += w(AB) * g(d), d the hops from the
        // destination to B (the sender); d = 0 counts as 1 (repo choice).
        const int d = std::max(static_cast<int>(h.hops) - 1, 1);
        const Zone z = zoneFor(from, dest).value_or(Zone::Two);
        const double g = config_.depositScale / (static_cast<double>(d) * d);
        pheromone_.add(dest, from, config_.weight[index(z)] * g);
        if (h.source == self_) {
            auto it = sources_.find(dest);
            if (it != sources_.end() && h.zone < kZones) {
                std::deque<int>& w = it->second.window[h.zone];
                w.push_back(h.hops);
                while (w.size() > config_.window) {
                    w.pop_front();
                }
            }
            return out;  // [K07t] Alg. 1: the backward ant is dropped at the source
        }
        auto br = backRouting_.find({h.source, h.seq});
        if (br == backRouting_.end()) {
            return out;
        }
        const NodeAddress previous = br->second.previous;
        backRouting_.erase(br);
        Header back = h;
        back.hops = static_cast<std::uint16_t>(h.hops + 1);
        out.push_back({Action::Kind::Send, previous, back});
        return out;
    }
    case PacketType::Data: {
        if (dest == self_) {
            out.push_back({Action::Kind::Deliver, self_, h});
            return out;
        }
        touch(dest, now);
        const bool fresh = dataSeen_.record(h.source, h.seq);
        if (!fresh && mode_ == Mode::Broken) {
            // [K07t] §5.5's loop guard: a packet seen twice in broken mode.
            noRouteToAll(dest, out);
            return out;
        }
        if (h.hops >= config_.maxHops) {
            return out;
        }
        const NodeAddress next = choose(dest, std::nullopt);
        if (next != kInvalidAddress) {
            Header data = h;
            data.hops = static_cast<std::uint16_t>(h.hops + 1);
            out.push_back({Action::Kind::Send, next, data});
        }
        return out;
    }
    case PacketType::NoRoute: {
        // [K07t] §5.5: as if the link to `from` were broken, for `dest`.
        pheromone_.erase(dest, from);
        if (!hasTrail(dest)) {
            lostTrail(dest, out);
        }
        return out;
    }
    }
    return out;
}

std::vector<Action> Router::linkDown(NodeAddress neighbour, Time now) {
    std::vector<Action> out;
    mode_ = Mode::Broken;
    brokenUntil_ = now + config_.brokenHold;
    for (NodeAddress dest : pheromone_.destinations()) {
        if (!pheromone_.contains(dest, neighbour)) {
            continue;
        }
        pheromone_.erase(dest, neighbour);
        if (!hasTrail(dest)) {
            lostTrail(dest, out);
        }
    }
    return out;
}

void Router::restart(NodeAddress dest) {
    SourceState& s = sources_[dest];
    for (auto& w : s.window) {
        w.clear();
    }
    s.open = {true, true, true};
    pheromone_.eraseDestination(dest);
}

void Router::lostTrail(NodeAddress dest, std::vector<Action>& out) {
    // [K07t] §5.5: a source starts route establishment again and suspends
    // its data; any other node tells its neighbours it has no route.
    if (sources_.contains(dest) && sources_.at(dest).queued > 0) {
        restart(dest);
        return;
    }
    noRouteToAll(dest, out);
}

void Router::noRouteToAll(NodeAddress dest, std::vector<Action>& out) const {
    for (NodeAddress n : neighbours()) {
        Header h;
        h.type = PacketType::NoRoute;
        h.source = self_;
        h.destination = dest;
        h.hops = 1;
        out.push_back({Action::Kind::Send, n, h});
    }
}

std::pair<double, double> Router::meanAndDeviation(const std::deque<int>& window) {
    if (window.empty()) {
        return {0.0, 0.0};
    }
    double sum = 0.0;
    for (int v : window) {
        sum += v;
    }
    const double mean = sum / static_cast<double>(window.size());
    double sq = 0.0;
    for (int v : window) {
        sq += (v - mean) * (v - mean);
    }
    return {mean, std::sqrt(sq / static_cast<double>(window.size()))};
}

bool Router::converged(NodeAddress dest, Zone zone) const {
    auto it = sources_.find(dest);
    if (it == sources_.end()) {
        return false;
    }
    const std::deque<int>& w = it->second.window[index(zone)];
    // An empty window is not converged (repo choice).
    return !w.empty() && meanAndDeviation(w).second < config_.sigmaThreshold;
}

bool Router::zoneOpen(NodeAddress dest, Zone zone) const {
    auto it = sources_.find(dest);
    return it == sources_.end() || it->second.open[index(zone)];
}

std::size_t Router::queued(NodeAddress dest) const {
    auto it = sources_.find(dest);
    return it == sources_.end() ? 0 : it->second.queued;
}

double Router::pheromone(NodeAddress dest, NodeAddress neighbour) const {
    return pheromone_.get(dest, neighbour);
}

void Router::closeLongerZones(NodeAddress dest, SourceState& state) {
    // [K07t] §5.4: sigma_i, sigma_j < t and alpha_j > alpha_i + c closes
    // zone j, and the source's trails in it are removed.
    for (int i = 0; i < kZones; ++i) {
        const auto zi = static_cast<Zone>(i);
        if (!state.open[i] || !converged(dest, zi)) {
            continue;
        }
        const double ai = meanAndDeviation(state.window[i]).first;
        for (int j = 0; j < kZones; ++j) {
            const auto zj = static_cast<Zone>(j);
            if (j == i || !state.open[j] || !converged(dest, zj)) {
                continue;
            }
            if (meanAndDeviation(state.window[j]).first > ai + config_.zoneTolerance) {
                state.open[j] = false;
                for (NodeAddress n : neighbours()) {
                    if (zoneFor(n, dest).value_or(Zone::Two) == zj) {
                        pheromone_.erase(dest, n);
                    }
                }
            }
        }
    }
}

void Router::sourceTick(NodeAddress dest, SourceState& state, std::vector<Action>& out) {
    if (pheromone_.row(dest).empty()) {
        initialise(dest);
    }
    closeLongerZones(dest, state);
    for (int i = 0; i < kZones; ++i) {
        const auto z = static_cast<Zone>(i);
        if (!state.open[i]) {
            continue;
        }
        if (converged(dest, z)) {
            // [K07t] §5.4: a converged zone sends data instead of ants.
            if (state.queued == 0) {
                continue;
            }
            const NodeAddress next = choose(dest, z);
            if (next == kInvalidAddress) {
                continue;
            }
            Header h;
            h.type = PacketType::Data;
            h.zone = static_cast<std::uint8_t>(i);
            h.source = self_;
            h.destination = dest;
            h.seq = nextSeq_++;
            h.hops = 1;
            dataSeen_.record(self_, h.seq);
            --state.queued;
            out.push_back({Action::Kind::Send, next, h});
            continue;
        }
        // [K07t] §5.3: n forward ants from each zone per interval.
        for (int k = 0; k < config_.antsPerZone; ++k) {
            const NodeAddress next = choose(dest, z);
            if (next == kInvalidAddress) {
                break;
            }
            Header h;
            h.type = PacketType::Forward;
            h.zone = static_cast<std::uint8_t>(i);
            h.source = self_;
            h.destination = dest;
            h.seq = nextSeq_++;
            h.hops = 1;
            // An ant that loops back to its source is a repeated ant there too.
            rememberBackRoute(self_, h.seq, {self_, dest});
            out.push_back({Action::Kind::Send, next, h});
        }
    }
}

std::vector<Action> Router::tick(Time now) {
    std::vector<Action> out;
    if (mode_ == Mode::Broken && now >= brokenUntil_) {
        mode_ = Mode::Regular;
    }
    for (auto& [dest, state] : sources_) {
        if (state.queued > 0) {
            touch(dest, now);
            sourceTick(dest, state, out);
        }
    }
    // [K07t] eq. 5: phi = mu * phi each interval.
    pheromone_.evaporate(config_.evaporation, 0.0);
    // [K07t] §5.2: an idle destination's trails and back routes go.
    for (auto it = lastUse_.begin(); it != lastUse_.end();) {
        if (now - it->second <= config_.trailTimeout) {
            ++it;
            continue;
        }
        const NodeAddress dest = it->first;
        pheromone_.eraseDestination(dest);
        std::erase_if(backRouting_, [dest](const auto& e) { return e.second.dest == dest; });
        std::erase_if(backRoutingOrder_,
                      [this](const auto& key) { return !backRouting_.contains(key); });
        sources_.erase(dest);
        it = lastUse_.erase(it);
    }
    return out;
}

}  // namespace posant::core
