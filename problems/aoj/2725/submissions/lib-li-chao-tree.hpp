#pragma once
#include "common.hpp"
#include "mylib/optimization/LiChaoTree.hpp"

// 2 乗のまま曲線として Li Chao 木に入れる。展開しないので式が素直になるが、
// 追加も取得も O(log C) かかる。
struct Solver {
  struct Curve {
    i64 operator()(i64 x, i64 a, i64 b) const {
      i64 d = x - a;
      return b - d * d;
    }
  };

  using Tree = LiChaoTree<Curve, std::tuple<i64, i64, i64>>;

  int t;
  vector<array<i64, 3>> items;
  i64 ans = 0;

  Solver(int t, const vector<array<i64, 3>> &items) : t(t), items(items) {}

  void run() {
    auto ord = order_by_time(items);
    Tree tree(Curve{});
    vector lcts(t + 1, tree.make_tree<MAXIMIZE>());
    ans = -(i64)1e9;
    for (int i = 0; i < (int)items.size(); ++i) {
      i64 ti = items[ord[i]][0], p = items[ord[i]][1], f = items[ord[i]][2];
      for (int x = t; x >= ti; --x) {
        i64 val = lcts[x - ti].query(f).first;
        val = std::max((i64)0, val) + p;
        ans = std::max(ans, val);
        lcts[x].insert(f, val);
      }
    }
  }

  i64 answer() const { return ans; }
};
