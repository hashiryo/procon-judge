#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/monge/monotone_minima.hpp"

// 区間 [l, r] に学校 1 校を置く費用 w(l, r) を先に全部作る (子の数の重み付き中央値)。
// 各層は dp[t][i] = min_{t-1 <= j < i} dp[t-1][j] + w(j+1, i) で、w が Monge なので argmin は
// i について単調。行の有効な列が階段になるので、範囲を渡せる
// monotone_minima_bounded_values で引き、返った値をそのまま次の層の dp にする。
// 層 t は j' = j - (t-1) に詰める (j' = 0 が j = t-1)。行 r は i = t + r で、列 j' は j' <= r。
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
    vector<i64> dp(n + 1, INF);
    dp[0] = 0;  // t = 0 は先頭 0 軒だけ
    for (int t = 1; t <= k; ++t) {
      const int base = t - 1, h = n + 1 - t;  // 行 r は先頭 i = t + r 軒、列 j' は j = base + j'
      auto value = [&](int r, int jj) -> i64 {
        const int i = t + r;
        return dp[jj] + w[(size_t)(i - 1) * n + (base + jj)];
      };
      auto lo = [&](int) { return 0; };
      auto hi = [&](int r) { return r + 1; };
      dp = monotone_minima_bounded_values<i64>(h, value, lo, hi);
    }
    ans = dp[n - k];  // 層 k の添字 r は、先頭 k + r 軒。n 軒のとき r = n - k。
  }

  i64 answer() const { return ans; }
};
