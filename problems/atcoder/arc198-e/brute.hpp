#pragma once
// 愚直解 (pj testdata crosscheck 用)。x = 0 から 2^N までの各 x について、各 s_i で (x OR s_i) + 1 へ数を配る (N <= 6)。
#include "pj.hpp"
struct Solver {
  static constexpr u64 P = 998244353;
  int n;
  vector<int> s;
  u32 ans = 0;
  Solver(int n_, const vector<int> &s_) : n(n_), s(s_) {}
  void run() {
    const size_t N = size_t(1) << n;
    vector<u64> dp(N + 1, 0);
    dp[0] = 1;
    for (size_t x = 0; x < N; ++x)
      for (int v : s) {
        const size_t y = (x | size_t(v)) + 1;
        if (y <= N) dp[y] = (dp[y] + dp[x]) % P;
      }
    ans = u32(dp[N]);
  }
  u32 answer() const { return ans; }
};
