// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only

#include "posant-header.h"

#include "swarm-wire.h"

namespace posant::core {

std::vector<std::uint8_t> encode(const Header& h) {
    std::vector<std::uint8_t> out;
    swarm::core::WireWriter w(out);
    w.reserve(kHeaderSize);
    w.version(kWireVersion);
    w.u8(static_cast<std::uint8_t>(h.type));
    w.u8(h.zone);
    w.i32(h.source);
    w.i32(h.destination);
    w.u32(h.seq);
    w.u16(h.hops);
    return out;
}

std::optional<Header> decode(std::span<const std::uint8_t> bytes) {
    swarm::core::WireReader r(bytes);
    if (!r.expectVersion(kWireVersion)) {
        return std::nullopt;
    }
    Header h;
    const std::uint8_t type = r.u8();
    if (type < static_cast<std::uint8_t>(PacketType::Forward) ||
        type > static_cast<std::uint8_t>(PacketType::NoRoute)) {
        r.fail();
    }
    h.type = static_cast<PacketType>(type);
    h.zone = r.u8();
    if (h.zone > 2) {
        r.fail();
    }
    h.source = r.i32();
    h.destination = r.i32();
    h.seq = r.u32();
    h.hops = r.u16();
    if (!r.atEnd()) {
        return std::nullopt;
    }
    return h;
}

}  // namespace posant::core
