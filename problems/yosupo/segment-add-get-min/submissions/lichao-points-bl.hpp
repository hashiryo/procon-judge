#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// lichao-points と同じ木で、線分を覆う節ごとの追加を分岐を使わずに葉まで下る形 (add_segment_full) にし、線分の端と
// 問いの葉を分岐のない二分探索で引く。
struct Solver {
  LiChaoPoints t;

  Solver(const vector<array<i64, 4>> &segs, const vector<i64> &xs) : t(xs) {
    for (auto &e : segs) t.add_segment_full(e[0], e[1], e[2], e[3]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add_segment_full(l, r, a, b); }

  i64 query(i64 p) { return t.query_bl(p); }
};
