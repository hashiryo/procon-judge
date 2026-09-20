#pragma once
#include "common.hpp"
#include "mylib/optimization/ConvexHullTrick.hpp"

// 2 乗を展開して直線の族に直してから、上側凸包を持つ。追加する直線の傾きが
// 単調なので凸包の更新が軽い。
struct Solver {
  int t;
  vector<array<i64, 3>> items;
  i64 ans = 0;

  Solver(int t, const vector<array<i64, 3>> &items) : t(t), items(items) {}

  void run() {
    auto ord = order_by_time(items);
    vector<ConvexHullTrick<i64, MAXIMIZE>> cht(t + 1);
    ans = -(i64)1e9;
    for (int i = 0; i < (int)items.size(); ++i) {
      i64 ti = items[ord[i]][0], p = items[ord[i]][1], f = items[ord[i]][2];
      for (int x = t; x >= ti; --x) {
        i64 val = p;
        if (!cht[x - ti].empty()) val = std::max(val, cht[x - ti].query(f) + p - f * f);
        ans = std::max(ans, val);
        cht[x].insert(2 * f, val - f * f);
      }
    }
  }

  i64 answer() const { return ans; }
};
