#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
struct Solver {
  using Tree = SplayTree<int>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  // 3 つに割って、真ん中をさらに 2 つに割り、順を入れ替えて繋ぎ直す。
  void rotate(int b, int m, int e) {
    auto [l, c, r] = t.split3(b, e);
    auto [cl, cr] = c.split(m - b);
    t = l + cr + cl + r;
  }

  vector<i64> dump() { return to_i64(t.dump()); }
};
