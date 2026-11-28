#pragma once
#include "pj.hpp"

// 愚直解。各層で全部の j を見る O(k n^2) の DP。n <= 40 の入力 (gen.py の seed 1000 以上) でだけ使う。
// base のハーネスに差し込むので、main ではなくハーネスの口 (Solver) を実装する。
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
      fill(ndp.begin(), ndp.end(), INF);
      for (int i = t; i <= n; ++i)
        for (int j = t - 1; j < i; ++j) {
          const i64 d = s[i] - s[j];
          ndp[i] = min(ndp[i], dp[j] + d * d);
        }
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
