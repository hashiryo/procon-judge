#pragma once
// _shared/mulfunc/min25.hpp の min_25 篩。今の Library の multiplicative_sum と同じ算法で、
// 篩の添字を掛け算と double の割り算で作り、mod を Barrett にしたもの。
#include "common.hpp"
#include "_shared/mulfunc/min25.hpp"

struct Solver {
  u64 n, a, b;
  u32 ans = 0;
  mf_min25::Core core;

  Solver(u64 n_, u64 a_, u64 b_) : n(n_), a(a_), b(b_) {}

  void run() {
    core.build(n, (u32)(a % MF_MOD), (u32)(b % MF_MOD), MF_MOD);
    ans = core.solve();
  }

  u64 answer() const { return ans; }
};
