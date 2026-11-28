#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// 直線 y = f_j X + dp[j] を Li Chao 木に足しながら、点 s_i での最小を引く。
// 傾きも問いも単調でないので、NeoLibrary の Li Chao 木 (x の範囲 [0, 1000001)) を使う。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f) {}

  void run() {
    LiChaoTree<i64> lct(0, 1000001, n + 1);
    lct.add_line(x, 0);  // まだ 1 体も倒していない状態
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = lct.query(s[i]);
      lct.add_line(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }
};
