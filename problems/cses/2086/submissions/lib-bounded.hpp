#pragma once
#include "pj.hpp"
#include "_shared/monge/monotone_minima_bounded.hpp"

// 行の有効な範囲を渡せる分割統治 (problems/_shared/monge/monotone_minima_bounded.hpp) で各層の最小を引く。
// 行 i の有効な列は [t-1, i) なので、範囲の外のセルを評価しない。
struct Solver {
  int n, k;
  vector<i64> s;  // s[i] = 先頭 i 個の和
  i64 ans = 0;

  Solver(const vector<i64> &a, int k) : n((int)a.size()), k(k), s(n + 1) {
    for (int i = 0; i < n; ++i) s[i + 1] = s[i] + a[i];
  }

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<i64> dp(n + 1, INF), ndp(n + 1, INF);
    dp[0] = 0;
    for (int t = 1; t <= k; ++t) {
      const int h = n + 1 - t;  // 行 r は i = t + r
      auto value = [&](int r, int j) -> i64 {
        const int i = t + r;
        const i64 d = s[i] - s[j];
        return dp[j] + d * d;
      };
      auto lo = [&](int) { return t - 1; };
      auto hi = [&](int r) { return t + r; };
      auto best = monotone_minima_bounded<i64>(h, value, lo, hi);
      for (int r = 0; r < h; ++r) ndp[t + r] = value(r, best[r]);
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
