#pragma once
#include "pj.hpp"
#include "_shared/monge/monotone_minima_bounded.hpp"

// 行の有効な範囲を渡せる分割統治の、値だけを返す形を、そのまま次の層の dp にする。
// 層 t の dp は、添字を j' = j - (t-1) に詰める (j' = 0 が j = t-1)。行 r の有効な列は j' <= r なので、
// lo(r) = 0、hi(r) = r+1。返った列がそのまま次の層の dp になり、写し直さない。
struct Solver {
  int n, k;
  vector<i64> s;  // s[i] = 先頭 i 個の和
  i64 ans = 0;

  Solver(const vector<i64> &a, int k) : n((int)a.size()), k(k), s(n + 1) {
    for (int i = 0; i < n; ++i) s[i + 1] = s[i] + a[i];
  }

  void run() {
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    vector<i64> dp(n + 1, INF);
    dp[0] = 0;
    for (int t = 1; t <= k; ++t) {
      const int base = t - 1, h = n + 1 - t;  // 行 r は i = t + r、列 j' は j = base + j'
      auto value = [&](int r, int jj) -> i64 {
        const int i = t + r, j = base + jj;
        const i64 d = s[i] - s[j];
        return dp[jj] + d * d;
      };
      auto lo = [&](int) { return 0; };
      auto hi = [&](int r) { return r + 1; };
      dp = monotone_minima_bounded_values<i64>(h, value, lo, hi);
    }
    ans = dp[n - k];  // 層 k の添字 r は、先頭 k + r 個。n 個のとき r = n - k。
  }

  i64 answer() const { return ans; }
};
