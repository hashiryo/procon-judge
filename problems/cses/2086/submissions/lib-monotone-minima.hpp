#pragma once
#include "pj.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// 今の Library の monotone_minima (比較の述語を受ける分割統治) で各層の最小を引く。
// 述語は 1 候補あたり 2 回の評価 (候補と今の最良) になる。
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
      auto cost = [&](int i, int j) -> i64 {
        const i64 d = s[i] - s[j];
        return dp[j] + d * d;
      };
      auto select = [&](int r, int j, int m) -> bool {  // m のほうがよいか
        const int i = t + r;
        if (j >= i) return true;   // j は無効
        if (m >= i) return false;  // m は無効
        return cost(i, m) < cost(i, j);
      };
      auto best = monotone_minima(h, n, select);
      for (int r = 0; r < h; ++r) ndp[t + r] = cost(t + r, best[r]);
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
