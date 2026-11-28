#pragma once
#include "pj.hpp"
#include "mylib/optimization/ConvexHullTrick.hpp"

// 今の Library の ConvexHullTrick (multiset に直線を持ち、交点で包絡を保つ) に
// 直線 y = f_j X + dp[j] を足しながら、点 s_i での最小を引く。傾きの順を問わないので、
// 2084 の単調性を使っていない。土台の比較用。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f) {}

  void run() {
    ConvexHullTrick<i64, MINIMIZE> cht;
    cht.insert(x, 0);  // まだ 1 体も倒していない状態
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = cht.query(s[i]);
      cht.insert(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }
};
