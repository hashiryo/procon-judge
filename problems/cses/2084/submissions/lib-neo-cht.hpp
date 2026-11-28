#pragma once
#include "pj.hpp"
#include "neo/algo/optimize/line/MonotoneCHT.hpp"

// 直線 y = f_j X + dp[j] を傾きの減る順 (f_0 = x >= f_1 >= …) に足し、問い s_i を増える順に
// query_monotone で引く。NeoLibrary の両端キューの CHT。
struct Solver {
  int n;
  i64 x;
  vector<i64> s, f;
  i64 ans = 0;

  Solver(const vector<i64> &s, const vector<i64> &f, i64 x)
      : n((int)s.size()), x(x), s(s), f(f) {}

  void run() {
    MonotoneCHT<i64> cht;
    cht.reserve(n + 1);
    cht.add_line(x, 0);  // まだ 1 体も倒していない状態
    i64 last = 0;
    for (int i = 0; i < n; ++i) {
      last = cht.query_monotone(s[i]);
      cht.add_line(f[i], last);
    }
    ans = last;  // dp[n]
  }

  i64 answer() const { return ans; }
};
