#pragma once
// トーシェントの和の 5 回目 (核は _tot5.hpp)。L = 0.8 N^{2/3}、再帰の双曲線の分け目は max(R/q, 1.0 √v)。
#include "_tot5.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot5::totient_sum<8, 100>((u64)n); }
    i64 answer() const { return ans; }
};
