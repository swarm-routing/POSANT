// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// One POSANT node, simulator-free (AGENTS.md rule 10): v0 from secondary
// sources, [K07t] ch. 5 (docs/fidelity.md). The adapter feeds it received
// headers, link failures and a periodic tick, and carries out the actions it
// returns. Positions come through a `Locate` function until swarm-core has
// its location port (swarm-routing/swarm-core#71); neighbours through
// swarm-core's INeighborProvider, randomness through its IRng.

#ifndef POSANT_ROUTER_H
#define POSANT_ROUTER_H

#include "posant-config.h"
#include "posant-header.h"
#include "posant-zone.h"
#include "swarm-dedup-history.h"
#include "swarm-pheromone-table.h"
#include "swarm-ports.h"

#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace posant::core {

using swarm::core::NodeAddress;
using swarm::core::Time;

/// A node's position, or nothing when it is not known.
using Locate = std::function<std::optional<Vec3>(NodeAddress)>;

/// What the adapter does next.
struct Action {
    enum class Kind : std::uint8_t {
        Send,     ///< send `header` to the neighbour `to`
        Deliver,  ///< `header` is a data packet for this node
    };
    Kind kind = Kind::Send;
    NodeAddress to = swarm::core::kInvalidAddress;
    Header header;

    bool operator==(const Action&) const = default;
};

/// [K07t] §5.5's two modes.
enum class Mode : std::uint8_t { Regular, Broken };

class Router {
public:
    Router(NodeAddress self, const Config& config, swarm::core::IRng& rng,
           const swarm::core::INeighborProvider& neighbours, Locate locate);

    NodeAddress self() const { return self_; }

    /// Queues one data packet for `dest` at this node, the source. False when
    /// the queue is full (the packet is refused) or `dest` is this node.
    bool sendData(NodeAddress dest, Time now);

    /// A header received from the neighbour `from`.
    std::vector<Action> receive(NodeAddress from, const Header& header, Time now);

    /// The link to `neighbour` broke ([K07t] §5.5).
    std::vector<Action> linkDown(NodeAddress neighbour, Time now);

    /// One interval ([K07t] Alg. 1): the source's ants or data, evaporation,
    /// idle destinations and the broken-mode hold.
    std::vector<Action> tick(Time now);

    // Read-only state, for tests and observers.
    double pheromone(NodeAddress dest, NodeAddress neighbour) const;
    Mode mode() const { return mode_; }
    /// True when the source's zone for `dest` has converged (sigma < t).
    bool converged(NodeAddress dest, Zone zone) const;
    /// True while the source still uses the zone for `dest`.
    bool zoneOpen(NodeAddress dest, Zone zone) const;
    /// Data packets queued at this source for `dest`.
    std::size_t queued(NodeAddress dest) const;
    std::size_t backRoutingSize() const { return backRouting_.size(); }

private:
    struct BackRoute {
        NodeAddress previous = swarm::core::kInvalidAddress;
        NodeAddress dest = swarm::core::kInvalidAddress;
    };
    struct SourceState {
        std::array<std::deque<int>, kZones> window;
        std::array<bool, kZones> open{true, true, true};
        std::size_t queued = 0;
    };

    std::vector<NodeAddress> neighbours() const;
    std::optional<Zone> zoneFor(NodeAddress neighbour, NodeAddress dest) const;
    void initialise(NodeAddress dest);
    bool hasTrail(NodeAddress dest) const;
    NodeAddress choose(NodeAddress dest, std::optional<Zone> onlyZone);
    void touch(NodeAddress dest, Time now);
    void rememberBackRoute(NodeAddress source, std::uint32_t seq, BackRoute route);
    void restart(NodeAddress dest);
    void lostTrail(NodeAddress dest, std::vector<Action>& out);
    void noRouteToAll(NodeAddress dest, std::vector<Action>& out) const;
    void sourceTick(NodeAddress dest, SourceState& state, std::vector<Action>& out);
    void closeLongerZones(NodeAddress dest, SourceState& state);
    static std::pair<double, double> meanAndDeviation(const std::deque<int>& window);

    NodeAddress self_;
    Config config_;
    swarm::core::IRng& rng_;
    const swarm::core::INeighborProvider& neighbours_;
    Locate locate_;

    swarm::core::PheromoneTable pheromone_;
    std::map<NodeAddress, Time> lastUse_;
    std::map<std::pair<NodeAddress, std::uint32_t>, BackRoute> backRouting_;
    std::deque<std::pair<NodeAddress, std::uint32_t>> backRoutingOrder_;
    swarm::core::DedupHistory dataSeen_;
    std::map<NodeAddress, SourceState> sources_;
    std::uint32_t nextSeq_ = 0;
    Mode mode_ = Mode::Regular;
    Time brokenUntil_ = 0.0;
};

}  // namespace posant::core

#endif  // POSANT_ROUTER_H
