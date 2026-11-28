#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/monge/monotone_minima.hpp"

// 各層の最小を NeoLibrary の monotone_minima (値の関数を受ける分割統治) で引く。
// 行 i の有効な列は j < i。dp[j] は t-1 個に分けたときの費用で、j < t-1 は到達しない (INF)。
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
      const int h = n + 1 - t;  // 行 t..n
      auto value = [&](int r, int j) -> i64 {
        const int i = t + r;
        if (j >= i) return INF;
        const i64 d = s[i] - s[j];
        return dp[j] + d * d;
      };
      auto best = monotone_minima<i64>(h, n, value);
      for (int r = 0; r < h; ++r) ndp[t + r] = value(r, best[r]);
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
