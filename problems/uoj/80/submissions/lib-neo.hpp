#pragma once
// NeoLibrary の assignment を、完全でなくてよい形 (perfect = false) の最大化で呼ぶ。組めない組は 0 の行列にする。
#include "neo/algo/optimize/matching/assignment.hpp"

struct Solver {
  int nl, nr;
  const vector<array<i64, 3>> &e;
  Assignment<i64> r;
  Solver(int nl, int nr, const vector<array<i64, 3>> &e) : nl(nl), nr(nr), e(e) {}
  void run() {
    vector<i64> a((size_t)nl * nr, 0);
    for (auto &[v, u, w] : e) a[(size_t)v * nr + u] = w;
    r = ::assignment(nl, nr, a, true, false);
  }
  i64 sum() const { return r.cost; }
  const vector<int> &mate() const { return r.col; }
};
