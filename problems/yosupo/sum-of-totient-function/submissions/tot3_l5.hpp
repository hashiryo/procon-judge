#pragma once
// トーシェントの和の 3 回目 (核は _tot3.hpp)。L = 0.5 N^{2/3}。
#include "_tot3.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot3::totient_sum<5>((u64)n); }
    i64 answer() const { return ans; }
};
