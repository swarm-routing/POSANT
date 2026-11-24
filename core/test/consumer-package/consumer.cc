// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The scratch consumer of the installed posant-core package: the headers resolve
// from include/posant-core/, and posant::core links.
#include "posant-version.h"

#include <cstdio>

int main() {
    std::printf("consumer: ok (posant-core %.*s)\n",
                static_cast<int>(posant::core::version().size()), posant::core::version().data());
    return 0;
}
