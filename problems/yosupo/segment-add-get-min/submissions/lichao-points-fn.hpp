#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao_fn.hpp"

// lichao-points と同じ手順を、載せる関数の型をテンプレートで受ける核 (LiChaoPointsFn) に直線 (LineFn) を渡して動かす。
struct Solver {
  LiChaoPointsFn<LineFn> t;

  Solver(const vector<array<i64, 4>> &segs, const vector<i64> &xs) : t(xs) {
    for (auto &e : segs) t.add(LineFn{e[2], e[3]}, e[0], e[1]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add(LineFn{a, b}, l, r); }

  i64 query(i64 p) { return t.query(p); }
};
