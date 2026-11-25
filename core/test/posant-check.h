// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The core tests' minimal harness: CHECK counts failures, each test file
// registers its functions with POSANT_TEST.

#ifndef POSANT_TEST_CHECK_H
#define POSANT_TEST_CHECK_H

#include <cstdio>
#include <functional>
#include <utility>
#include <vector>

namespace posant::test {

inline int& failures() {
    static int n = 0;
    return n;
}

struct Registered {
    const char* name;
    std::function<void()> fn;
};

inline std::vector<Registered>& registry() {
    static std::vector<Registered> tests;
    return tests;
}

struct Register {
    Register(const char* name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

}  // namespace posant::test

#define POSANT_CHECK(cond)                                                                         \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: FAIL: %s\n", __FILE__, __LINE__, #cond);                  \
            ++posant::test::failures();                                                            \
        }                                                                                          \
    } while (false)

#define POSANT_CAT2(a, b) a##b
#define POSANT_CAT(a, b) POSANT_CAT2(a, b)
#define POSANT_TEST(name)                                                                          \
    static void name();                                                                            \
    static const posant::test::Register POSANT_CAT(reg_, name){#name, name};                       \
    static void name()

#endif  // POSANT_TEST_CHECK_H
