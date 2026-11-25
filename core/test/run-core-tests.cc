// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// Runs every registered core test; nonzero exit on any failure.

#include "posant-check.h"
#include "posant-version.h"

#include <cstdio>

POSANT_TEST(versionIsThePackageVersion) {
    POSANT_CHECK(posant::core::version() == POSANT_CORE_VERSION_STRING);
    POSANT_CHECK(!posant::core::version().empty());
}

int main() {
    for (const auto& t : posant::test::registry()) {
        const int before = posant::test::failures();
        t.fn();
        std::printf("%s %s\n", posant::test::failures() == before ? "ok  " : "FAIL", t.name);
    }
    if (posant::test::failures() != 0) {
        std::fprintf(stderr, "%d check(s) failed\n", posant::test::failures());
        return 1;
    }
    std::puts("posant-core: ok");
    return 0;
}
