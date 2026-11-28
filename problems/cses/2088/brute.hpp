#pragma once
#include "pj.hpp"

// 愚直解。dp[l][r] = min_{l <= k < r} dp[l][k] + dp[k+1][r] + sum(l, r) を、長さの短い順に
// k を全部見て求める O(n^3)。n <= 60 の入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  int n;
  vector<i64> pre;
  vector<i64> dp;  // dp[l * n + r]
  i64 ans = 0;

  explicit Solver(const vector<i64> &x) : n((int)x.size()), pre(n + 1), dp((size_t)n * n, 0) {
    for (int i = 0; i < n; ++i) pre[i + 1] = pre[i] + x[i];
  }

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    if (n <= 1) {
      ans = 0;
      return;
    }
    for (int len = 2; len <= n; ++len)
      for (int l = 0; l + len <= n; ++l) {
        const int r = l + len - 1;
        i64 best = INF;
        for (int k = l; k < r; ++k) best = min(best, dp[(size_t)l * n + k] + dp[(size_t)(k + 1) * n + r]);
        dp[(size_t)l * n + r] = best + (pre[r + 1] - pre[l]);
      }
    ans = dp[n - 1];  // dp[0 * n + (n - 1)]
  }

  i64 answer() const { return ans; }
};
