// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// POSANT's zones ([K07t] §5.1): a node's neighbours split by the angle between
// the vector to the neighbour and the vector to the destination. In 3D the
// same angle gives cones about the destination's direction (a repo choice,
// docs/fidelity.md); with equal heights it is the 2D rule.

#ifndef POSANT_ZONE_H
#define POSANT_ZONE_H

#include <cstdint>

namespace posant::core {

/// A position. z = 0 throughout gives the 2D algorithm.
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    bool operator==(const Vec3&) const = default;
};

/// zone 1 (index 0): theta <= pi/4; zone 2 (1): pi/4 < theta < 3pi/4;
/// zone 3 (2): theta >= 3pi/4.
enum class Zone : std::uint8_t { One = 0, Two = 1, Three = 2 };

inline constexpr int kZones = 3;

/// The zone of `neighbour` seen from `self` for a destination at `dest`. A
/// zero-length vector (the neighbour or the destination at `self`'s position)
/// gives zone 1 (repo choice).
Zone zoneOf(const Vec3& self, const Vec3& neighbour, const Vec3& dest);

inline int index(Zone z) {
    return static_cast<int>(z);
}

}  // namespace posant::core

#endif  // POSANT_ZONE_H
