#pragma once
// トーシェントの和の 4 回目 (核は _tot4.hpp)。L = 1.2 N^{2/3}。
#include "_tot4.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot4::totient_sum<12>((u64)n); }
    i64 answer() const { return ans; }
};
