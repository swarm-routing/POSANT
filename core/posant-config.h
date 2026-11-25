// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// POSANT's parameters. v0 from secondary sources (docs/fidelity.md, AGENTS.md
// rule 11): each default names its row in docs/fidelity.md's Parameters table.

#ifndef POSANT_CONFIG_H
#define POSANT_CONFIG_H

#include "swarm-types.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace posant::core {

struct Config {
    /// (nu1, nu2, nu3): initial pheromone by zone ([K07t] Table 1, p. 42).
    std::array<double, 3> initialPheromone{20.0, 1.0, 1.0};
    /// (w1, w2, w3): backward-ant weight by zone ([K07t] eq. 4 bounds them;
    /// the values are a repo choice).
    std::array<double, 3> weight{1.0, 1.0, 1.0};
    /// mu: pheromone kept per tick ([K07t] Table 1, eq. 5).
    double evaporation = 0.95;
    /// t: the standard-deviation threshold of convergence ([K07t] Table 1).
    double sigmaThreshold = 1.0;
    /// The window of backward-ant delays per zone ([K07t] Table 1).
    std::size_t window = 50;
    /// g(d) = depositScale / d^2 ([K07t] Table 1).
    double depositScale = 1000.0;
    /// n: forward ants per zone per tick ([K07t] §5.3).
    int antsPerZone = 1;
    /// c: route-length tolerance between zones, in hops (repo choice).
    double zoneTolerance = 2.0;
    /// Trails of an idle destination are dropped after this (repo choice).
    swarm::core::Time trailTimeout = 5.0;
    /// Broken mode lasts this long after a link failure (repo choice).
    swarm::core::Time brokenHold = 0.05;
    /// Forward ants and data packets are dropped after this many hops (repo choice).
    std::uint16_t maxHops = 64;
    /// Bounds (org rule 7; repo choices).
    std::size_t pheromoneCapacity = 4096;
    std::size_t backRoutingCapacity = 1024;
    std::size_t dataHistoryCapacity = 1024;
    std::size_t sourceQueue = 64;
};

}  // namespace posant::core

#endif  // POSANT_CONFIG_H
