#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao_fn.hpp"

// lichao-range と同じ手順を、載せる関数の型をテンプレートで受ける核 (LiChaoRangeFn) に直線 (LineFn) を渡して動かす。
struct Solver {
  LiChaoRangeFn<LineFn> t;

  explicit Solver(const vector<array<i64, 4>> &segs) : t(-1000000000, 1000000001, 4 * ((int)segs.size() + 200000)) {
    for (auto &e : segs) t.add(LineFn{e[2], e[3]}, e[0], e[1]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add(LineFn{a, b}, l, r); }

  i64 query(i64 p) { return t.query(p); }
};
