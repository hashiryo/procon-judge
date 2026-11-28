#pragma once
#include "pj.hpp"
#include "mylib/optimization/monotone_minima.hpp"

// 今の Library の monotone_minima (比較の述語を受ける分割統治) で各層の最小を引く。
// 述語は 1 候補あたり 2 回の評価 (候補と今の最良) になる。行 i の有効な列は j < i なので、
// 列が i 以上のセルは「必ず負ける」として扱う。
struct Solver {
  int n, k;
  vector<i64> sc, sci;
  vector<i64> w;  // w[r * n + l] = 区間 [l, r] に学校 1 校の費用
  i64 ans = 0;

  Solver(const vector<i64> &c, int k)
      : n((int)c.size()), k(k), sc(n + 1), sci(n + 1), w((size_t)n * n) {
    for (int i = 0; i < n; ++i) sc[i + 1] = sc[i] + c[i], sci[i + 1] = sci[i] + c[i] * i;
    for (int l = 0; l < n; ++l) {
      int m = l;
      for (int r = l; r < n; ++r) {
        const i64 total = sc[r + 1] - sc[l];
        while (m < r && 2 * (sc[m + 1] - sc[l]) < total) ++m;
        const i64 left = m * (sc[m + 1] - sc[l]) - (sci[m + 1] - sci[l]);
        const i64 right = (sci[r + 1] - sci[m + 1]) - m * (sc[r + 1] - sc[m + 1]);
        w[(size_t)r * n + l] = left + right;
      }
    }
  }

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<i64> dp(n + 1, INF), ndp(n + 1, INF);
    dp[0] = 0;  // t = 0 は先頭 0 軒だけ
    for (int t = 1; t <= k; ++t) {
      fill(ndp.begin(), ndp.end(), INF);
      const int h = n + 1 - t;  // 行 r は先頭 i = t + r 軒
      auto cost = [&](int i, int j) -> i64 {
        if (j >= i) return INF;  // 列 j は使えない
        return dp[j] + w[(size_t)(i - 1) * n + j];
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
