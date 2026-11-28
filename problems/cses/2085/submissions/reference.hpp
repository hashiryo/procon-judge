#pragma once
#include "pj.hpp"

// 想定解。直線 y = f_j X + dp[j] を Li Chao 木に足しながら、問い s_i での最小を引く。
// j = 0 は「まだ 1 体も倒していない」状態で、f_0 = x、dp[0] = 0。答えは dp[n]。
// s_i <= 1e6 なので、x の範囲を [0, 2^20) に取った完全二分木に直線を置く。
// 期待出力を作る側なので、Library を include しない。
struct Solver {
  static constexpr i64 INF = (i64)4e18;
  static constexpr int W = 1 << 20;  // x の範囲 [0, W)

  int n;
  i64 x;
  vector<i64> s, f;
  vector<char> used;  // 節に直線が載っているか
  vector<i64> la, lb;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f), used(2 * W), la(2 * W), lb(2 * W, INF) {}

  void run() {
    add_line(x, 0);
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = query(s[i]);
      add_line(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }

private:
  void add_line(i64 na, i64 nb) {
    int k = 1, l = 0, r = W;
    for (;;) {
      if (!used[k]) {
        used[k] = 1, la[k] = na, lb[k] = nb;
        return;
      }
      const int m = (l + r) >> 1;
      const bool wl = na * l + nb < la[k] * l + lb[k];
      const bool wm = na * m + nb < la[k] * m + lb[k];
      if (wm) swap(na, la[k]), swap(nb, lb[k]);
      if (r - l == 1) return;
      if (wl != wm) k = 2 * k, r = m;
      else k = 2 * k + 1, l = m;
    }
  }

  i64 query(i64 X) const {
    i64 res = INF;
    int k = 1, l = 0, r = W;
    for (;;) {
      if (used[k]) res = min(res, la[k] * X + lb[k]);
      if (r - l == 1) break;
      const int m = (l + r) >> 1;
      if (X < m) k = 2 * k, r = m;
      else k = 2 * k + 1, l = m;
    }
    return res;
  }
};
