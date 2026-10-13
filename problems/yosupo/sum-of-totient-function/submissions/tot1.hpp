#pragma once
// トーシェントの和の 1 回目 (核は _tot1.hpp)。Mertens 関数の床の値を、N^{2/3} の半分までの μ の篩と、大きい値の再帰で求める。
#include "_tot1.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = tot1::totient_sum((u64)n); }
    i64 answer() const { return ans; }
};
