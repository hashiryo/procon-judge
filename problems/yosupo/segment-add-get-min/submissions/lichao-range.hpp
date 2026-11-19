#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// x の範囲 [-1e9, 1e9] を渡す Li Chao 木 (LiChaoRange)。線分は範囲を覆う節に分けてから、それぞれの節の下で直線を足す
// ように下る。
struct Solver {
  LiChaoRange t;

  explicit Solver(const vector<array<i64, 4>> &segs) : t(-1000000000, 1000000001, 4 * ((int)segs.size() + 200000)) {
    for (auto &e : segs) t.add_segment(e[0], e[1], e[2], e[3]);
  }

  void insert(i64 l, i64 r, i64 a, i64 b) { t.add_segment(l, r, a, b); }

  i64 query(i64 p) { return t.query(p); }
};
