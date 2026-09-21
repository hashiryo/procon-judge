#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized_Undoable.hpp"

// 巻き戻せるポテンシャル付き Union-Find。履歴を残すために経路圧縮ができない
// ので、根まで登る距離が O(log N) になる。この問題では巻き戻しを使わないので、
// 経路圧縮を捨てたぶんがそのまま差になる。
struct Solver {
  int m;
  vector<i64> l, r;
  bool ok = false;

  Solver(int m, const vector<i64> &l, const vector<i64> &r) : m(m), l(l), r(r) {}

  void run() {
    const int n = (int)l.size();
    UnionFind_Potentialized_Undoable<bool> uf(n);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < i; ++j) {
        auto [same, differ] = constraint(m, l[i], r[i], l[j], r[j]);
        if (differ && !uf.unite(i, j, true)) return;
        if (same && !uf.unite(i, j, false)) return;
      }
    ok = true;
  }

  bool answer() const { return ok; }
};
