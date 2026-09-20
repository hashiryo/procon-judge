#pragma once
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
struct Solver {
  using Tree = SplayTree<int, true>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  void reverse(int b, int e) { t.reverse(b, e); }

  vector<i64> dump() { return to_i64(t.dump()); }
};
