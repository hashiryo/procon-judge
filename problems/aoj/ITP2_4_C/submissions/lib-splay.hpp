#pragma once
#include <utility>
#include "common.hpp"
#include "mylib/data_structure/SplayTree.hpp"

// スプレー木。触った位置を根へ回転で持ち上げるので、木の形が操作列に依存する。
struct Solver {
  using Tree = SplayTree<int>;

  Tree t;

  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}

  // 後ろの区間から先に切り出す。先に前を抜くと後ろの位置がずれる。
  void swap_ranges(int b, int e, int t) {
    int f = t + e - b;
    if (t < b) std::swap(b, t), std::swap(e, f);
    auto [tl, tc, tr] = this->t.split3(t, f);
    auto [bl, bc, br] = tl.split3(b, e);
    this->t = bl + tc + br + bc + tr;
  }

  vector<i64> dump() { return to_i64(t.dump()); }
};
