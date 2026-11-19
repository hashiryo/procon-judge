#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// 問いの x を先に受け取り、座標を詰めた Li Chao 木 (LiChaoPoints) に入れる。問いは二分探索で葉の添字を引いてから、
// 葉から根へ上る。
struct Solver {
  LiChaoPoints t;

  Solver(const vector<array<i64, 2>> &lines, const vector<i64> &xs) : t(xs) {
    for (auto &e : lines) t.add_line(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64 p) { return t.query(p); }
};
