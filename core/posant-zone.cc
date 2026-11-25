// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only

#include "posant-zone.h"

#include <cmath>
#include <numbers>

namespace posant::core {

Zone zoneOf(const Vec3& self, const Vec3& neighbour, const Vec3& dest) {
    const double ax = neighbour.x - self.x;
    const double ay = neighbour.y - self.y;
    const double az = neighbour.z - self.z;
    const double bx = dest.x - self.x;
    const double by = dest.y - self.y;
    const double bz = dest.z - self.z;
    const double na = std::sqrt(ax * ax + ay * ay + az * az);
    const double nb = std::sqrt(bx * bx + by * by + bz * bz);
    if (na == 0.0 || nb == 0.0) {
        return Zone::One;
    }
    // Compare cosines, not angles: theta <= pi/4 iff cos(theta) >= cos(pi/4).
    const double cosTheta = (ax * bx + ay * by + az * bz) / (na * nb);
    const double quarter = std::numbers::sqrt2 / 2.0;
    // A rounding margin, so that a neighbour exactly on a zone boundary
    // (cos = +-sqrt(2)/2 in exact arithmetic) lands on the side [K07t] §5.1
    // closes: zone 1 at pi/4, zone 3 at 3pi/4.
    const double eps = 1e-12;
    if (cosTheta >= quarter - eps) {
        return Zone::One;
    }
    if (cosTheta > -quarter + eps) {
        return Zone::Two;
    }
    return Zone::Three;
}

}  // namespace posant::core
