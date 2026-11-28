#pragma once
#include "common.hpp"
#include "neo/algo/optimize/monge/larsch.hpp"

// 費用行列が Monge であることを使う LARSCH (NeoLibrary の larsch_dp)。行を 1 つずつ確定させながら
// 降りるので、費用関数を O(N log N) 回しか呼ばない。
struct Solver {
  vector<i64> a, x, y;
  i64 ans = 0;

  Solver(const vector<i64> &a, const vector<i64> &x, const vector<i64> &y)
      : a(a), x(x), y(y) {}

  void run() {
    const int n = (int)a.size();
    auto w = [&](int i, int j) { return pen(x[j] - a[i - 1]) + pen(y[j]); };
    ans = larsch_dp<i64>(n, w)[n];
  }

  i64 answer() const { return ans; }
};
