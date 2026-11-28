#pragma once
#include "pj.hpp"

// 各層の最小を分割統治で求める。行 i の有効な列は j < i なので、候補の範囲を i で切り落とす。
// 費用 (S_i - S_j)^2 は S が広義単調増加なら Monge なので、行の最小の列は行について単調になる。
// 期待出力を作る側なので、Library を include しない (ライブラリを直しても期待出力は変わらない)。
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
      // 行 i in [t, n]、列 j in [t-1, i-1]。
      auto rec = [&](auto &rec, int h1, int h2, int w1, int w2) -> void {
        if (h1 == h2) return;
        const int hm = (h1 + h2) / 2;
        const int wr = w2 < hm ? w2 : hm;
        int bj = w1;
        i64 bv = dp[bj] + (s[hm] - s[bj]) * (s[hm] - s[bj]);
        for (int j = w1 + 1; j < wr; ++j) {
          const i64 d = s[hm] - s[j], v = dp[j] + d * d;
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
