#pragma once
// トーシェントの和の 6 回目 (核は _tot6.hpp)。L = 1.6 N^{2/3}、30 と互いに素な数だけを篩い、再帰の分け目は max(R/q, 0.4 √v)。
#include "_tot6.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot6::totient_sum<16, 40>((u64)n); }
    i64 answer() const { return ans; }
};
