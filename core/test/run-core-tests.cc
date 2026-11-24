// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The skeleton's tests: posant-core builds, reports its version, and links
// swarm-core's core/ (the dependency every protocol slice builds on). The
// protocol's own tests come with #38.

#include "posant-version.h"
#include "swarm-dedup-history.h"

#include <cstdio>

namespace {

int failures = 0;

void check(bool ok, const char* what) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

}  // namespace

int main() {
    check(posant::core::version() == POSANT_CORE_VERSION_STRING,
          "version() is the package version");
    check(!posant::core::version().empty(), "version() is not empty");

    swarm::core::DedupHistory history(4);
    check(history.record(1, 7), "swarm-core links: a first (source, seq) is new");
    check(!history.record(1, 7), "swarm-core links: a repeat is a duplicate");

    if (failures == 0) {
        std::puts("posant-core: ok");
    }
    return failures == 0 ? 0 : 1;
}
