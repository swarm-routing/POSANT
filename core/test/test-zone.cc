// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// [K07t] §5.1's zones, their boundaries, and the 3D repo choice.

#include "posant-check.h"
#include "posant-zone.h"

#include <cmath>
#include <numbers>

using posant::core::Vec3;
using posant::core::Zone;
using posant::core::zoneOf;

namespace {

Vec3 at(double angle) {
    return {std::cos(angle), std::sin(angle), 0.0};
}

}  // namespace

POSANT_TEST(zonesByAngle) {
    const Vec3 self{0, 0, 0};
    const Vec3 dest{10, 0, 0};
    const double pi = std::numbers::pi;
    POSANT_CHECK(zoneOf(self, at(0.0), dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, at(pi / 6), dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, at(-pi / 6), dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, at(pi / 3), dest) == Zone::Two);
    POSANT_CHECK(zoneOf(self, at(pi / 2), dest) == Zone::Two);
    POSANT_CHECK(zoneOf(self, at(-2 * pi / 3), dest) == Zone::Two);
    POSANT_CHECK(zoneOf(self, at(5 * pi / 6), dest) == Zone::Three);
    POSANT_CHECK(zoneOf(self, at(pi), dest) == Zone::Three);
}

POSANT_TEST(zoneBoundariesAreClosedAsTheThesisSays) {
    // theta = pi/4 is zone 1 (theta <= pi/4); theta = 3pi/4 is zone 3.
    const Vec3 self{0, 0, 0};
    const Vec3 dest{10, 0, 0};
    POSANT_CHECK(zoneOf(self, {1, 1, 0}, dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, {-1, 1, 0}, dest) == Zone::Three);
}

POSANT_TEST(zoneIgnoresDistance) {
    const Vec3 self{5, 5, 0};
    const Vec3 dest{5, 100, 0};
    POSANT_CHECK(zoneOf(self, {5.1, 6, 0}, dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, {5.1, 60, 0}, dest) == Zone::One);
}

POSANT_TEST(zoneDegenerateVectorsAreZoneOne) {
    const Vec3 p{1, 2, 3};
    POSANT_CHECK(zoneOf(p, p, {4, 5, 6}) == Zone::One);
    POSANT_CHECK(zoneOf(p, {4, 5, 6}, p) == Zone::One);
}

POSANT_TEST(zones3dAreConesAndEqualHeightIs2d) {
    const Vec3 self{0, 0, 0};
    const Vec3 dest{0, 0, 10};  // straight up
    POSANT_CHECK(zoneOf(self, {0.1, 0, 1}, dest) == Zone::One);
    POSANT_CHECK(zoneOf(self, {1, 1, 0}, dest) == Zone::Two);
    POSANT_CHECK(zoneOf(self, {0, 0.2, -1}, dest) == Zone::Three);
    // Every node at height 7: the same zones as in the plane.
    for (int k = 0; k < 16; ++k) {
        const double a = k * std::numbers::pi / 8 + 0.01;
        const Vec3 n2 = at(a);
        const Vec3 n3{n2.x, n2.y, 7.0};
        POSANT_CHECK(zoneOf({0, 0, 0}, n2, {10, 0, 0}) == zoneOf({0, 0, 7}, n3, {10, 0, 7}));
    }
}
