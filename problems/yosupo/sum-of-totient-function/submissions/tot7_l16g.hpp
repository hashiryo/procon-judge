#pragma once
// トーシェントの和の 7 回目 (核は _tot7.hpp)。totient_sum<16, 100, true>。
#include "_tot7.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot7::totient_sum<16, 100, true>((u64)n); }
    i64 answer() const { return ans; }
};
