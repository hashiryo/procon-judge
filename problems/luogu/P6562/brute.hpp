#pragma once
// 愚直解 (pj testdata crosscheck 用)。a ⊆ c ⊆ b の c を全部たどって足す。
#include "pj.hpp"
struct Solver {
  vector<u32> a;
  vector<array<u32, 4>> ops;
  vector<u32> ans;
  Solver(int, const vector<u32> &a_, const vector<array<u32, 4>> &ops_) : a(a_), ops(ops_) {}
  void run() {
    for (auto &o : ops) {
      const u32 x = o[1], y = o[2];
      u32 s = 0;
      if ((x & y) == x)
        for (u32 t = x ^ y;; t = (t - 1) & (x ^ y)) {
          if (o[0] == 1) a[t | x] += o[3];
          else s += a[t | x];
          if (!t) break;
        }
      if (o[0] == 2) ans.push_back(s);
    }
  }
  vector<u32> answer() const { return ans; }
};
