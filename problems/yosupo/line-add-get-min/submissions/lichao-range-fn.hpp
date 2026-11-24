#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao_fn.hpp"

// lichao-range と同じ手順を、載せる関数の型をテンプレートで受ける核 (LiChaoRangeFn) に直線 (LineFn) を渡して動かす。
// 関数の型を受ける形にしたときの遅れを見る。
struct Solver {
  LiChaoRangeFn<LineFn> t;

  explicit Solver(const vector<array<i64, 2>> &lines) : t(-1000000000, 1000000001, (int)lines.size() + 200000) {
    for (auto &e : lines) t.add(LineFn{e[0], e[1]});
  }

  void insert(i64 a, i64 b) { t.add(LineFn{a, b}); }

  i64 query(i64 p) { return t.query(p); }
};
