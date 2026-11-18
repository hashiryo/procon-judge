#pragma once
// 愚直解 (pj testdata crosscheck 用)。exp(f)[S] = sum_{S の集合の分割} prod_{塊 B} f[B] を、分割を全部たどって求める (n <= 6)。
#include "pj.hpp"
struct Solver {
  int n;
  vector<u64> f, h;
  Solver(int n_, const vector<u64> &f_) : n(n_), f(f_) {}
  // rest を、最小の要素を含む塊から順に分ける
  u64 go(u32 rest) const {
    if (!rest) return 1;
    const u32 lo = rest & -rest, r = rest ^ lo;
    u64 t = 0;
    for (u32 a = r;; a = (a - 1) & r) {
      t += f[lo | a] * go(r ^ a);
      if (!a) break;
    }
    return t;
  }
  void run() {
    h.assign(size_t(1) << n, 0);
    for (u32 s = 0; s < (1u << n); ++s) h[s] = go(s);
  }
  vector<u64> answer() const { return h; }
};
