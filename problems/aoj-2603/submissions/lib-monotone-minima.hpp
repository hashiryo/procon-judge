#pragma once
#include "common.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// 組の数を 1 つずつ増やしながら、各段で totally monotone な行列の行最小を
// 分割統治で取る。段が m 回あるので O(m N log N)。
struct Solver {
  int m;
  vector<i64> src;
  i64 ans = 0;

  Solver(int m, const vector<i64> &src) : m(m), src(src) {}

  void run() {
    const int n = (int)src.size();
    if (n <= m) {
      ans = 0;
      return;
    }
    Cost w(src);
    vector<i64> dp(n + 1, (i64)1e9);
    dp[0] = 0;
    for (int t = m; t--;) {
      auto select = [&](int i, int j, int k) {
        return dp[j] + w(i, j) > dp[k] + w(i, k);
      };
      auto id = monotone_minima(n + 1, n + 1, select);
      for (int i = n; i > 0; --i) dp[i] = dp[id[i]] + w(i, id[i]);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
