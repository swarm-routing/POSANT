// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only

#include "posant-version.h"

namespace posant::core {

std::string_view version() noexcept {
    return POSANT_CORE_VERSION_STRING;
}

}  // namespace posant::core
