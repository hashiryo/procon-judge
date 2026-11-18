#pragma once
// 愚直解 (pj testdata crosscheck 用)。log(f)[S] = sum_{S の集合の分割 π} (-1)^{|π|-1} (|π|-1)! prod_{塊 B} f[B] を、分割を全部たどって
// 求める (n <= 6、S = ∅ は 0)。
#include "pj.hpp"
struct Solver {
  int n;
  vector<u64> f, g;
  Solver(int n_, const vector<u64> &f_) : n(n_), f(f_) {}
  // rest を分けた塊の数 k ごとの prod の和 (cnt[k])
  void go(u32 rest, int k, u64 prod, vector<u64> &cnt) const {
    if (!rest) {
      cnt[k] += prod;
      return;
    }
    const u32 lo = rest & -rest, r = rest ^ lo;
    for (u32 a = r;; a = (a - 1) & r) {
      go(r ^ a, k + 1, prod * f[lo | a], cnt);
      if (!a) break;
    }
  }
  void run() {
    g.assign(size_t(1) << n, 0);
    for (u32 s = 1; s < (1u << n); ++s) {
      vector<u64> cnt(n + 2, 0);
      go(s, 0, 1, cnt);
      u64 t = 0, fact = 1;  // fact = (k-1)!
      for (int k = 1; k <= n; ++k) {
        t += (k & 1 ? cnt[k] : u64(0) - cnt[k]) * fact;
        fact *= u64(k);
      }
      g[s] = t;
    }
  }
  vector<u64> answer() const { return g; }
};
