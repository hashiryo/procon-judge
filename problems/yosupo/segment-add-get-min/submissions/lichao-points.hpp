#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// 問いの x を先に受け取り、座標を詰めた Li Chao 木 (LiChaoPoints) に入れる。線分の端は二分探索で葉の添字の区間に
// 直し、問いも二分探索で葉を引いてから根へ上る。
struct Solver {
  LiChaoPoints t;

  Solver(const vector<array<i64, 4>> &segs, const vector<i64> &xs) : t(xs) {
    for (auto &e : segs) t.add_segment(e[0], e[1], e[2], e[3]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add_segment(l, r, a, b); }

  i64 query(i64 p) { return t.query(p); }
};
