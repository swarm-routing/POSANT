// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// POSANT's one control and data header (org rule 3): a POD, little-endian,
// with a leading version byte. The fields are the ones [K07t] ch. 5 names
// (type, source, destination, sequence number, path length) plus the source
// zone an ant or data packet left from; the layout is a repo choice.

#ifndef POSANT_HEADER_H
#define POSANT_HEADER_H

#include "swarm-types.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace posant::core {

inline constexpr std::uint8_t kWireVersion = 1;

enum class PacketType : std::uint8_t {
    Forward = 1,   ///< forward ant (FANT)
    Backward = 2,  ///< backward ant (BANT)
    Data = 3,
    NoRoute = 4,  ///< "no route to destination from me" ([K07t] §5.5)
};

struct Header {
    PacketType type = PacketType::Forward;
    /// The source's zone (0..2) the packet left from.
    std::uint8_t zone = 0;
    swarm::core::NodeAddress source = swarm::core::kInvalidAddress;
    swarm::core::NodeAddress destination = swarm::core::kInvalidAddress;
    std::uint32_t seq = 0;
    /// Hops made on arrival at the receiver: forward ants and data count from
    /// the source, backward ants from the destination.
    std::uint16_t hops = 0;

    bool operator==(const Header&) const = default;
};

/// Bytes on the wire: version, type, zone, source, destination, seq, hops.
inline constexpr std::size_t kHeaderSize = 1 + 1 + 1 + 4 + 4 + 4 + 2;

std::vector<std::uint8_t> encode(const Header& h);
/// The header, or nothing for a wrong version, an unknown type, a zone above
/// 2, too few bytes or trailing bytes.
std::optional<Header> decode(std::span<const std::uint8_t> bytes);

}  // namespace posant::core

#endif  // POSANT_HEADER_H
