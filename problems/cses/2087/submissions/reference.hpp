#pragma once
#include "pj.hpp"

// 想定解。区間 [l, r] に学校を 1 校置く費用 w(l, r) を先に全部作る。子の数の重み付き中央値を m とすると
// w(l, r) = sum_{i=l}^{m} c_i (m - i) + sum_{i=m+1}^{r} c_i (i - m) で、累積和と c_i × 位置の累積和から O(1)。
// 各層 dp[t][i] = min_{j < i} dp[t-1][j] + w(j+1, i) は、w が Monge なので argmin が i について単調。
// 行 [h1, h2) と列 [w1, w2) を受ける分割統治で求める。
// 期待出力を作る側なので、Library を include しない。
struct Solver {
  int n, k;
  vector<i64> sc, sci;  // 子の数の累積和と、子の数 × 位置の累積和
  vector<i64> w;        // w[r * n + l] = 区間 [l, r] に学校 1 校の費用
  i64 ans = 0;

  Solver(const vector<i64> &c, int k)
      : n((int)c.size()), k(k), sc(n + 1), sci(n + 1), w((size_t)n * n) {
    for (int i = 0; i < n; ++i) sc[i + 1] = sc[i] + c[i], sci[i + 1] = sci[i] + c[i] * i;
    for (int l = 0; l < n; ++l) {
      int m = l;  // 重み付き中央値。r を増やしても左へは戻らない
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
    vector<i64> dp(n + 1, INF);
    dp[0] = 0;  // t = 0 は先頭 0 軒だけ
    for (int t = 1; t <= k; ++t) {
      vector<i64> ndp(n + 1, INF);
      // 行 i in [t, n]、列 j in [t-1, i-1]。
      auto rec = [&](auto &rec, int h1, int h2, int w1, int w2) -> void {
        if (h1 == h2) return;
        const int hm = (h1 + h2) / 2;
        const int wr = min(w2, hm);
        const i64 *row = &w[(size_t)(hm - 1) * n];
        int bj = w1;
        i64 bv = dp[w1] + row[w1];
        for (int j = w1 + 1; j < wr; ++j) {
          const i64 v = dp[j] + row[j];
          if (v < bv) bv = v, bj = j;
        }
        ndp[hm] = bv;
        rec(rec, h1, hm, w1, bj + 1);
        rec(rec, hm + 1, h2, bj, w2);
      };
      rec(rec, t, n + 1, t - 1, n);
      dp.swap(ndp);
    }
    ans = dp[n];
  }

  i64 answer() const { return ans; }
};
