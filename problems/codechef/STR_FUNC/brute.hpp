#pragma once
// 愚直解 (pj testdata crosscheck 用)。定義のとおり、全部の i と j の組を見る (N <= 64)。
#include "pj.hpp"
struct Solver {
  static constexpr u64 P = 1000000007;
  vector<int> a;
  u32 ans = 0;
  explicit Solver(const vector<int> &a_) : a(a_) {}
  void run() {
    const int n = int(a.size());
    vector<u64> F(n), G(n);
    u64 total = 0;
    for (int i = 0; i < n; ++i) {
      u64 sg = 0;
      for (int j = 0; j < i; ++j)
        if ((i & j) == j) sg = (sg + G[j]) % P;
      F[i] = (u64(a[i]) * a[i] + sg * sg) % P;
      u64 sf = 0;
      for (int j = 0; j <= i; ++j)
        if ((i & j) == j) sf = (sf + F[j] * F[j]) % P;
      G[i] = sf;
      total = (total + u64(i) * F[i] % P * G[i]) % P;
    }
    ans = u32(total);
  }
  u32 answer() const { return ans; }
};
