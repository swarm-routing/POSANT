// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// A tiny deterministic network for the core tests: nodes at fixed positions,
// links given explicitly (irregular ranges, as [K07t] ch. 2), one hop per
// step, no MAC. Every header goes through the wire codec on its way.

#ifndef POSANT_TEST_NET_H
#define POSANT_TEST_NET_H

#include "posant-router.h"

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace posant::test {

/// A seeded RNG port (org rule 5).
class TestRng : public swarm::core::IRng {
public:
    explicit TestRng(std::uint64_t seed) : gen_(seed) {}
    double uniform() override {
        // 53 random bits in [0, 1): the same on every standard library.
        return static_cast<double>(gen_() >> 11) * 0x1.0p-53;
    }
    int uniformInt(int n) override { return n <= 0 ? 0 : static_cast<int>(uniform() * n); }

private:
    std::mt19937_64 gen_;
};

class Net;

class NodeNeighbours : public swarm::core::INeighborProvider {
public:
    NodeNeighbours(const Net& net, core::NodeAddress self) : net_(net), self_(self) {}
    std::vector<core::NodeAddress> neighbors() const override;

private:
    const Net& net_;
    core::NodeAddress self_;
};

class Net {
public:
    explicit Net(std::vector<core::Vec3> positions, std::uint64_t seed = 1,
                 core::Config config = {})
        : positions_(std::move(positions)), rng_(seed) {
        for (std::size_t i = 0; i < positions_.size(); ++i) {
            const auto id = static_cast<core::NodeAddress>(i);
            providers_.push_back(std::make_unique<NodeNeighbours>(*this, id));
            routers_.push_back(std::make_unique<core::Router>(
                id, config, rng_, *providers_.back(), [this](core::NodeAddress n) {
                    return n >= 0 && static_cast<std::size_t>(n) < positions_.size()
                               ? std::optional<core::Vec3>(positions_[n])
                               : std::nullopt;
                }));
        }
    }

    void link(core::NodeAddress a, core::NodeAddress b) {
        links_.insert({std::min(a, b), std::max(a, b)});
    }
    bool linked(core::NodeAddress a, core::NodeAddress b) const {
        return links_.contains({std::min(a, b), std::max(a, b)});
    }
    /// Breaks a link: both ends learn it (as a MAC failure would tell them).
    void cut(core::NodeAddress a, core::NodeAddress b) {
        links_.erase({std::min(a, b), std::max(a, b)});
        enqueue(a, routers_[a]->linkDown(b, now_));
        enqueue(b, routers_[b]->linkDown(a, now_));
    }

    core::Router& node(core::NodeAddress n) { return *routers_[n]; }
    std::size_t size() const { return routers_.size(); }

    /// One interval: every router ticks, then messages hop until none is in
    /// flight (each hop is one step; a step budget guards against loops).
    void step() {
        now_ += 0.1;
        for (std::size_t i = 0; i < routers_.size(); ++i) {
            enqueue(static_cast<core::NodeAddress>(i), routers_[i]->tick(now_));
        }
        for (int hop = 0; hop < 256 && !inFlight_.empty(); ++hop) {
            std::vector<Msg> batch;
            batch.swap(inFlight_);
            for (const Msg& m : batch) {
                if (!linked(m.from, m.to)) {
                    continue;  // sent on a link that has since broken
                }
                const auto bytes = core::encode(m.header);
                const auto decoded = core::decode(bytes);
                if (!decoded) {
                    continue;
                }
                ++sent_;
                enqueue(m.to, routers_[m.to]->receive(m.from, *decoded, now_));
            }
        }
    }

    std::size_t delivered() const { return delivered_; }
    std::size_t sent() const { return sent_; }
    const std::vector<std::uint16_t>& deliveredHops() const { return hops_; }
    /// Every action, in order: a determinism fingerprint.
    const std::vector<core::Action>& trace() const { return trace_; }
    double now() const { return now_; }

private:
    friend class NodeNeighbours;
    struct Msg {
        core::NodeAddress from;
        core::NodeAddress to;
        core::Header header;
    };

    void enqueue(core::NodeAddress from, const std::vector<core::Action>& actions) {
        for (const core::Action& a : actions) {
            trace_.push_back(a);
            if (a.kind == core::Action::Kind::Deliver) {
                ++delivered_;
                hops_.push_back(a.header.hops);
            } else {
                inFlight_.push_back({from, a.to, a.header});
            }
        }
    }

    std::vector<core::Vec3> positions_;
    std::set<std::pair<core::NodeAddress, core::NodeAddress>> links_;
    TestRng rng_;
    std::vector<std::unique_ptr<NodeNeighbours>> providers_;
    std::vector<std::unique_ptr<core::Router>> routers_;
    std::vector<Msg> inFlight_;
    std::vector<core::Action> trace_;
    std::vector<std::uint16_t> hops_;
    std::size_t delivered_ = 0;
    std::size_t sent_ = 0;
    double now_ = 0.0;
};

inline std::vector<core::NodeAddress> NodeNeighbours::neighbors() const {
    std::vector<core::NodeAddress> out;
    for (const auto& [a, b] : net_.links_) {
        if (a == self_) {
            out.push_back(b);
        } else if (b == self_) {
            out.push_back(a);
        }
    }
    return out;
}

}  // namespace posant::test

#endif  // POSANT_TEST_NET_H
