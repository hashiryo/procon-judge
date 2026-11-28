#pragma once
#include "pj.hpp"

// 愚直解。区間 [l, r] に学校を 1 校置く費用を、置く家 p を全部試して O(n^3) で作り、
// dp[i] = min_{j < i} dp[j] + w(j+1, i) を各層で j を全部見て求める O(k n^2)。
// n <= 40 の入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  int n, k;
  vector<i64> sc, sci;
  vector<i64> w;
  i64 ans = 0;

  Solver(const vector<i64> &c, int k)
      : n((int)c.size()), k(k), sc(n + 1), sci(n + 1), w((size_t)n * n) {
    for (int i = 0; i < n; ++i) sc[i + 1] = sc[i] + c[i], sci[i + 1] = sci[i] + c[i] * i;
    for (int l = 0; l < n; ++l)
      for (int r = l; r < n; ++r) {
        i64 best = numeric_limits<i64>::max();
        for (int p = l; p <= r; ++p) {  // 学校を家 p に置く
          const i64 left = p * (sc[p + 1] - sc[l]) - (sci[p + 1] - sci[l]);
          const i64 right = (sci[r + 1] - sci[p + 1]) - p * (sc[r + 1] - sc[p + 1]);
          best = min(best, left + right);
        }
        w[(size_t)r * n + l] = best;
      }
  }

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<i64> dp(n + 1, INF), ndp(n + 1, INF);
    dp[0] = 0;
    for (int t = 1; t <= k; ++t) {
      fill(ndp.begin(), ndp.end(), INF);
      for (int i = t; i <= n; ++i) {
        i64 best = INF;
        for (int j = t - 1; j < i; ++j) best = min(best, dp[j] + w[(size_t)(i - 1) * n + j]);
        ndp[i] = best;
      }
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
