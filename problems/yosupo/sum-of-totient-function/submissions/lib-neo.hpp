#pragma once
// NeoLibrary の totient_sum を呼ぶ。中身は 7 回目 (_tot7.hpp) の tot7_l16c40g と同じ手順で、Φ(N) を __int128 で丸めずに
// 求めてから 998244353 で割る。
#include "common.hpp"
#include "neo/number_theory/totient_sum.hpp"

struct Solver {
    i64 n;
    u32 ans = 0;
    explicit Solver(i64 n) : n(n) {}
    void run() { ans = (u32)(totient_sum((unsigned long long)n) % 998244353); }
    i64 answer() const { return ans; }
};
