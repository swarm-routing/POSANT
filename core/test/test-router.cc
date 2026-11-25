// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// One router's rules ([K07t] §§5.2-5.5), then whole networks: delivery,
// a void greedy routing cannot cross, zone closing, link failure,
// determinism and the bounds.

#include "posant-check.h"
#include "posant-net.h"
#include "posant-router.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

using posant::core::Action;
using posant::core::Config;
using posant::core::Header;
using posant::core::NodeAddress;
using posant::core::PacketType;
using posant::core::Router;
using posant::core::Vec3;
using posant::core::Zone;
using posant::test::Net;
using posant::test::TestRng;

namespace {

class FixedNeighbours : public swarm::core::INeighborProvider {
public:
    explicit FixedNeighbours(std::vector<NodeAddress> n) : n_(std::move(n)) {}
    std::vector<NodeAddress> neighbors() const override { return n_; }

private:
    std::vector<NodeAddress> n_;
};

// Node 0 at the origin; 1 east (zone 1 toward 9), 2 north (zone 2),
// 3 west (zone 3); the destination 9 far east, not a neighbour.
std::map<NodeAddress, Vec3> star() {
    return {{0, {0, 0, 0}}, {1, {1, 0, 0}}, {2, {0, 1, 0}}, {3, {-1, 0, 0}}, {9, {10, 0, 0}}};
}

posant::core::Locate locator(std::map<NodeAddress, Vec3> pos) {
    return [pos = std::move(pos)](NodeAddress n) -> std::optional<Vec3> {
        auto it = pos.find(n);
        return it == pos.end() ? std::nullopt : std::optional<Vec3>(it->second);
    };
}

Header forward(NodeAddress source, NodeAddress dest, std::uint32_t seq) {
    Header h;
    h.type = PacketType::Forward;
    h.source = source;
    h.destination = dest;
    h.seq = seq;
    h.hops = 1;
    return h;
}

}  // namespace

POSANT_TEST(pheromoneStartsByZone) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    r.receive(3, forward(3, 9, 0), 0.0);
    POSANT_CHECK(r.pheromone(9, 1) == 20.0);
    POSANT_CHECK(r.pheromone(9, 2) == 1.0);
    POSANT_CHECK(r.pheromone(9, 3) == 1.0);
}

POSANT_TEST(nextHopFollowsEquationTwo) {
    // p_i = phi_i / sum phi: 20/22, 1/22, 1/22, the previous hop included.
    TestRng rng(7);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    std::map<NodeAddress, int> count;
    const int n = 22000;
    for (int i = 0; i < n; ++i) {
        const auto out = r.receive(3, forward(5, 9, static_cast<std::uint32_t>(i)), 0.0);
        POSANT_CHECK(out.size() == 1);
        if (!out.empty()) {
            ++count[out[0].to];
        }
    }
    POSANT_CHECK(std::abs(count[1] / double(n) - 20.0 / 22.0) < 0.01);
    POSANT_CHECK(std::abs(count[2] / double(n) - 1.0 / 22.0) < 0.01);
    POSANT_CHECK(std::abs(count[3] / double(n) - 1.0 / 22.0) < 0.01);
}

POSANT_TEST(repeatedForwardAntIsDestroyed) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    POSANT_CHECK(r.receive(3, forward(5, 9, 4), 0.0).size() == 1);
    POSANT_CHECK(r.receive(1, forward(5, 9, 4), 0.0).empty());
}

POSANT_TEST(destinationAnswersWithBackwardAnt) {
    TestRng rng(1);
    FixedNeighbours nb({1});
    auto pos = star();
    Router d(9, Config{}, rng, nb, locator(pos));
    Header f = forward(0, 9, 11);
    f.zone = 2;
    f.hops = 4;
    const auto out = d.receive(1, f, 0.0);
    POSANT_CHECK(out.size() == 1);
    if (!out.empty()) {
        POSANT_CHECK(out[0].to == 1);
        POSANT_CHECK(out[0].header.type == PacketType::Backward);
        POSANT_CHECK(out[0].header.seq == 11 && out[0].header.zone == 2);
        POSANT_CHECK(out[0].header.hops == 1);
    }
}

POSANT_TEST(backwardAntDepositsEquationThree) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Config c;
    c.weight = {2.0, 1.0, 0.5};
    Router r(0, c, rng, nb, locator(star()));
    // Make the back route: the forward ant came from 3.
    r.receive(3, forward(5, 9, 1), 0.0);
    Header b = forward(5, 9, 1);
    b.type = PacketType::Backward;
    b.hops = 1;  // from the destination itself: d = 0, counted as 1
    const double before = r.pheromone(9, 1);
    const auto out = r.receive(1, b, 0.0);
    // w1 * 1000 / 1^2 on the link to 1 (zone 1).
    POSANT_CHECK(r.pheromone(9, 1) == before + 2.0 * 1000.0);
    POSANT_CHECK(out.size() == 1 && out[0].to == 3 && out[0].header.hops == 2);
    // Three hops from the destination at B: d = 2, g = 250, zone 2's w = 1.
    r.receive(3, forward(5, 9, 2), 0.0);
    Header b2 = b;
    b2.seq = 2;
    b2.hops = 3;
    const double before2 = r.pheromone(9, 2);
    r.receive(2, b2, 0.0);
    POSANT_CHECK(r.pheromone(9, 2) == before2 + 250.0);
}

POSANT_TEST(evaporationMultipliesByMu) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    r.receive(3, forward(5, 9, 0), 0.0);
    r.tick(0.1);
    POSANT_CHECK(std::abs(r.pheromone(9, 1) - 20.0 * 0.95) < 1e-12);
}

POSANT_TEST(idleDestinationIsForgotten) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    r.receive(3, forward(5, 9, 0), 0.0);
    POSANT_CHECK(r.backRoutingSize() == 1);
    r.tick(4.0);
    POSANT_CHECK(r.pheromone(9, 1) > 0.0);
    r.tick(5.5);
    POSANT_CHECK(r.pheromone(9, 1) == 0.0);
    POSANT_CHECK(r.backRoutingSize() == 0);
}

POSANT_TEST(backRoutingTableIsBounded) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Config c;
    c.backRoutingCapacity = 16;
    Router r(0, c, rng, nb, locator(star()));
    for (std::uint32_t s = 0; s < 1000; ++s) {
        r.receive(3, forward(5, 9, s), 0.0);
        POSANT_CHECK(r.backRoutingSize() <= 16);
    }
}

POSANT_TEST(sourceQueueIsBounded) {
    TestRng rng(1);
    FixedNeighbours nb({1});
    Config c;
    c.sourceQueue = 3;
    Router r(0, c, rng, nb, locator(star()));
    POSANT_CHECK(r.sendData(9, 0.0) && r.sendData(9, 0.0) && r.sendData(9, 0.0));
    POSANT_CHECK(!r.sendData(9, 0.0));
    POSANT_CHECK(!r.sendData(0, 0.0));
    POSANT_CHECK(r.queued(9) == 3);
}

namespace {

// A w x h grid, spacing 1, 4-neighbour links.
Net grid(int w, int h, std::uint64_t seed) {
    std::vector<Vec3> pos;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            pos.push_back({double(x), double(y), 0.0});
        }
    }
    Net net(pos, seed);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int i = y * w + x;
            if (x + 1 < w) net.link(i, i + 1);
            if (y + 1 < h) net.link(i, i + w);
        }
    }
    return net;
}

void sendAll(Net& net, NodeAddress s, NodeAddress d, int packets, int steps) {
    for (int k = 0; k < packets; ++k) {
        net.node(s).sendData(d, net.now());
    }
    for (int k = 0; k < steps; ++k) {
        net.step();
    }
}

}  // namespace

POSANT_TEST(deliversAcrossAGrid) {
    Net net = grid(5, 5, 3);
    sendAll(net, 0, 24, 40, 300);
    POSANT_CHECK(net.delivered() == 40);
    POSANT_CHECK(net.node(0).queued(24) == 0);
    // The last packets take short routes (the shortest is 8 hops).
    const auto& h = net.deliveredHops();
    if (h.size() >= 10) {
        double sum = 0;
        for (std::size_t i = h.size() - 10; i < h.size(); ++i)
            sum += h[i];
        POSANT_CHECK(sum / 10.0 <= 12.0);
    }
}

POSANT_TEST(crossesAVoidGreedyRoutingCannot) {
    // S=0 at (0,0) and D=1 at (10,0). S's only zone-1 neighbour, 2, is a dead
    // end. The way round goes north: 0-3-4-5-6-1.
    std::vector<Vec3> pos = {{0, 0, 0}, {10, 0, 0}, {1, 0, 0}, {0, 3, 0},
                             {3, 5, 0}, {7, 5, 0},  {10, 3, 0}};
    Net net(pos, 5);
    net.link(0, 2);
    net.link(0, 3);
    net.link(3, 4);
    net.link(4, 5);
    net.link(5, 6);
    net.link(6, 1);
    sendAll(net, 0, 1, 20, 300);
    POSANT_CHECK(net.delivered() == 20);
}

POSANT_TEST(closesTheLongerZone) {
    // S=0 at (0,0), D=1 at (10,0). Zone 1: 2 at (1,0) starts a 7-hop chain;
    // zone 3: 3 at (-1,0) reaches D in 2 hops (an irregular, long link).
    std::vector<Vec3> pos = {{0, 0, 0}, {10, 0, 0}, {1, 0, 0}, {-1, 0, 0}};
    for (int k = 0; k < 5; ++k) {
        pos.push_back({2.0 + k, 1.0, 0.0});
    }
    Net net(pos, 9);
    net.link(0, 2);
    net.link(2, 4);
    for (int k = 4; k < 8; ++k)
        net.link(k, k + 1);
    net.link(8, 1);
    net.link(0, 3);
    net.link(3, 1);
    // Checked while the source is still sending: once its queue is empty and
    // the trail timeout passes, the destination's state is forgotten.
    sendAll(net, 0, 1, 64, 20);
    POSANT_CHECK(!net.node(0).zoneOpen(1, Zone::One));
    POSANT_CHECK(net.node(0).zoneOpen(1, Zone::Three));
    POSANT_CHECK(net.node(0).converged(1, Zone::Three));
    sendAll(net, 0, 1, 0, 100);
    POSANT_CHECK(net.delivered() == 64);
}

POSANT_TEST(recoversFromALinkFailure) {
    // Two disjoint routes S=0 to D=1: 0-2-3-1 and 0-4-5-1.
    std::vector<Vec3> pos = {{0, 0, 0},   {3, 0, 0},    {1, 0.5, 0},
                             {2, 0.5, 0}, {1, -0.5, 0}, {2, -0.5, 0}};
    Net net(pos, 11);
    net.link(0, 2);
    net.link(2, 3);
    net.link(3, 1);
    net.link(0, 4);
    net.link(4, 5);
    net.link(5, 1);
    sendAll(net, 0, 1, 20, 100);
    POSANT_CHECK(net.delivered() == 20);
    net.cut(2, 3);
    POSANT_CHECK(net.node(2).pheromone(1, 3) == 0.0);
    sendAll(net, 0, 1, 20, 200);
    POSANT_CHECK(net.delivered() >= 39);
    POSANT_CHECK(net.node(0).queued(1) == 0);
}

POSANT_TEST(noRouteWhenTheLastTrailGoes) {
    // [K07t] §5.5: a node left with no trail for D tells every neighbour.
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    r.receive(3, forward(5, 9, 0), 0.0);
    Header no;
    no.type = PacketType::NoRoute;
    no.destination = 9;
    POSANT_CHECK(r.receive(1, no, 0.0).empty());
    POSANT_CHECK(r.receive(2, no, 0.0).empty());
    const auto out = r.receive(3, no, 0.0);
    POSANT_CHECK(out.size() == 3);
    for (const Action& a : out) {
        POSANT_CHECK(a.header.type == PacketType::NoRoute && a.header.destination == 9 &&
                     a.header.source == 0);
    }
}

POSANT_TEST(sourceRestartsWhenItsLastTrailGoes) {
    TestRng rng(1);
    FixedNeighbours nb({1});
    Router r(0, Config{}, rng, nb, locator(star()));
    r.sendData(9, 0.0);
    r.tick(0.1);  // initialises and sends an ant
    POSANT_CHECK(r.pheromone(9, 1) > 0.0);
    const auto out = r.linkDown(1, 0.2);
    POSANT_CHECK(out.empty());  // a source restarts instead of answering no-route
    POSANT_CHECK(r.mode() == posant::core::Mode::Broken);
    POSANT_CHECK(r.queued(9) == 1);
    POSANT_CHECK(!r.converged(9, Zone::One));
}

POSANT_TEST(dataSeenTwiceInBrokenModeAnswersNoRoute) {
    TestRng rng(1);
    FixedNeighbours nb({1, 2, 3});
    Router r(0, Config{}, rng, nb, locator(star()));
    Header d;
    d.type = PacketType::Data;
    d.source = 5;
    d.destination = 9;
    d.seq = 3;
    d.hops = 1;
    POSANT_CHECK(r.receive(3, d, 0.0).size() == 1);
    // Regular mode: a repeat is still forwarded.
    POSANT_CHECK(r.receive(3, d, 0.0).size() == 1);
    r.linkDown(2, 0.0);
    const auto out = r.receive(1, d, 0.0);
    POSANT_CHECK(out.size() == 3);
    POSANT_CHECK(!out.empty() && out[0].header.type == PacketType::NoRoute);
}

POSANT_TEST(brokenModeEndsAfterTheHold) {
    Net net = grid(3, 3, 1);
    net.cut(0, 1);
    POSANT_CHECK(net.node(0).mode() == posant::core::Mode::Broken);
    net.step();
    POSANT_CHECK(net.node(0).mode() == posant::core::Mode::Regular);
}

POSANT_TEST(sameSeedSameRun) {
    Net a = grid(4, 4, 77);
    Net b = grid(4, 4, 77);
    sendAll(a, 0, 15, 25, 150);
    sendAll(b, 0, 15, 25, 150);
    POSANT_CHECK(a.trace() == b.trace());
    POSANT_CHECK(a.delivered() == 25);
}

POSANT_TEST(threeDimensionalNetworkDelivers) {
    // A 3 x 3 x 3 lattice: the zones are cones (repo choice).
    std::vector<Vec3> pos;
    for (int z = 0; z < 3; ++z)
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 3; ++x)
                pos.push_back({double(x), double(y), double(z)});
    Net net(pos, 13);
    auto id = [](int x, int y, int z) { return z * 9 + y * 3 + x; };
    for (int z = 0; z < 3; ++z)
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 3; ++x) {
                if (x < 2) net.link(id(x, y, z), id(x + 1, y, z));
                if (y < 2) net.link(id(x, y, z), id(x, y + 1, z));
                if (z < 2) net.link(id(x, y, z), id(x, y, z + 1));
            }
    sendAll(net, id(0, 0, 0), id(2, 2, 2), 30, 300);
    POSANT_CHECK(net.delivered() == 30);
}
