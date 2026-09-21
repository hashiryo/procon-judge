#pragma once
#include <algorithm>
#include "common.hpp"
#include "mylib/optimization/simplified_larsch_dp.hpp"

// 費用行列が Monge であることを使う LARSCH。費用関数を O(N log N) 回しか
// 呼ばない。
struct Solver {
  i64 a, b, w;
  vector<i64> d;
  i64 ans = 0;

  Solver(i64 a, i64 b, i64 w, const vector<i64> &d) : a(a), b(b), w(w), d(d) {}

  void run() {
    const i64 n = (i64)d.size();
    auto cost = [&](int i, int j) {
      return d[i - 1] + b * (i - j) * (i - j - 1) / 2 - a * (i - j - 1);
    };
    auto dp = simplified_larsch_dp((int)n, cost);
    ans = (i64)1e18;
    for (i64 i = 0; i <= n; ++i) ans = std::min(ans, dp[i] + tail(a, b, n, i));
    ans += w;
  }

  i64 answer() const { return ans; }
};
