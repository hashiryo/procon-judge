#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/MonotoneCHT.hpp"

// 費用を展開する。dp_t[i] = S_i^2 + min_j (dp_{t-1}[j] + S_j^2 - 2 S_j S_i)。
// S は広義単調増加なので、傾き -2 S_j は増えず、問い S_i は増える。NeoLibrary の MonotoneCHT で各層 O(n)。
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
      MonotoneCHT<i64> cht;
      cht.reserve(n + 1);
      for (int i = t; i <= n; ++i) {
        const int j = i - 1;  // 列 j を足してから i を問う
        cht.add_line(-2 * s[j], dp[j] + s[j] * s[j]);
        ndp[i] = s[i] * s[i] + cht.query_monotone(s[i]);
      }
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
