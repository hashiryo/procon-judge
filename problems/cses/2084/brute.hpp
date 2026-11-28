#pragma once
#include "pj.hpp"

// 愚直解。dp[i] = min_{0 <= j < i} (dp[j] + s_i f_j) を各 i で j を全部見て求める O(n^2)。
// j = 0 はまだ 1 体も倒していない状態 (f_0 = x、dp[0] = 0)。
// n <= 60 の入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f) {}

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<i64> dp(n + 1, INF);
    dp[0] = 0;
    for (int i = 1; i <= n; ++i) {
      i64 best = dp[0] + s[i - 1] * x;
      for (int j = 1; j < i; ++j) best = min(best, dp[j] + s[i - 1] * f[j - 1]);
      dp[i] = best;
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
