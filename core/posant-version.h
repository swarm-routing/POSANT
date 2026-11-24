// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The version of the posant-core package. core/ holds no protocol code yet:
// it waits for the review of docs/fidelity.md (AGENTS.md rule 11; #37, #38).

#ifndef POSANT_VERSION_H
#define POSANT_VERSION_H

#include <string_view>

namespace posant::core {

/// The package version posant-core was built as (.cz.toml's, or POSANT_CORE_VERSION).
std::string_view version() noexcept;

}  // namespace posant::core

#endif  // POSANT_VERSION_H
