#pragma once
#include <algorithm>
#include "common.hpp"
#include "neo/algo/optimize/line/LiChaoTree.hpp"

// NeoLibrary の LiChaoTree を最大で使い、2 乗のまま曲線 b - (x - a)^2 として載せる (今の Library の lib-li-chao-tree と
// 同じ解き方)。2 次の係数が同じなので、どの 2 本も高々 1 回しか交わらない。範囲は時刻の最小と最大から決める。
struct Solver {
  struct Curve {
    i64 a, b;
    i64 operator()(i64 x) const { return b - (x - a) * (x - a); }
  };

  int t;
  vector<array<i64, 3>> items;
  i64 ans = 0;

  Solver(int t, const vector<array<i64, 3>> &items) : t(t), items(items) {}

  void run() {
    auto ord = order_by_time(items);
    i64 lo = items[0][2], hi = items[0][2];
    for (auto &e : items) lo = std::min(lo, e[2]), hi = std::max(hi, e[2]);
    vector lcts(t + 1, LiChaoTree<i64, true, Curve>(lo, hi + 1));
    ans = -(i64)1e9;
    for (int i = 0; i < (int)items.size(); ++i) {
      i64 ti = items[ord[i]][0], p = items[ord[i]][1], f = items[ord[i]][2];
      for (int x = t; x >= ti; --x) {
        i64 val = lcts[x - ti].query(f);
        val = std::max((i64)0, val) + p;
        ans = std::max(ans, val);
        lcts[x].add(Curve{f, val});
      }
    }
  }

  i64 answer() const { return ans; }
};
