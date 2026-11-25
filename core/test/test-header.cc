// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The wire header (org rule 3): roundtrip, rejections, and a deterministic fuzz.

#include "posant-check.h"
#include "posant-header.h"
#include "posant-net.h"

#include <cstdint>
#include <vector>

using posant::core::decode;
using posant::core::encode;
using posant::core::Header;
using posant::core::kHeaderSize;
using posant::core::kWireVersion;
using posant::core::PacketType;

POSANT_TEST(headerRoundtrip) {
    for (auto type :
         {PacketType::Forward, PacketType::Backward, PacketType::Data, PacketType::NoRoute}) {
        Header h;
        h.type = type;
        h.zone = 2;
        h.source = -7;
        h.destination = 123456;
        h.seq = 0xDEADBEEFu;
        h.hops = 65535;
        const auto bytes = encode(h);
        POSANT_CHECK(bytes.size() == kHeaderSize);
        POSANT_CHECK(bytes[0] == kWireVersion);
        const auto back = decode(bytes);
        POSANT_CHECK(back.has_value() && *back == h);
    }
}

POSANT_TEST(headerIsLittleEndian) {
    Header h;
    h.seq = 0x01020304u;
    const auto b = encode(h);
    POSANT_CHECK(b[11] == 0x04 && b[12] == 0x03 && b[13] == 0x02 && b[14] == 0x01);
}

POSANT_TEST(headerRejectsBadInput) {
    Header h;
    auto good = encode(h);
    auto wrongVersion = good;
    wrongVersion[0] = kWireVersion + 1;
    POSANT_CHECK(!decode(wrongVersion));
    auto badType = good;
    badType[1] = 9;
    POSANT_CHECK(!decode(badType));
    auto zeroType = good;
    zeroType[1] = 0;
    POSANT_CHECK(!decode(zeroType));
    auto badZone = good;
    badZone[2] = 3;
    POSANT_CHECK(!decode(badZone));
    auto trailing = good;
    trailing.push_back(0);
    POSANT_CHECK(!decode(trailing));
    for (std::size_t n = 0; n < good.size(); ++n) {
        POSANT_CHECK(!decode(std::span<const std::uint8_t>(good.data(), n)));
    }
}

POSANT_TEST(headerFuzzNeverMisreads) {
    // Random bytes either fail to decode or decode to a header that encodes
    // back to exactly those bytes.
    posant::test::TestRng rng(42);
    for (int i = 0; i < 20000; ++i) {
        const int len = rng.uniformInt(static_cast<int>(kHeaderSize) + 3);
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(len));
        for (auto& b : bytes) {
            b = static_cast<std::uint8_t>(rng.uniformInt(256));
        }
        if (len > 0 && rng.uniformInt(2) == 0) {
            bytes[0] = kWireVersion;
        }
        const auto h = decode(bytes);
        if (h) {
            POSANT_CHECK(encode(*h) == bytes);
        }
    }
}
