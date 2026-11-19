#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// x の範囲 [-1e9, 1e9] を渡す Li Chao 木 (LiChaoRange)。節は使う所だけ作り、配列の添字で持つ。追加も問いも根から
// 1 本の道を下る。
struct Solver {
  LiChaoRange t;

  explicit Solver(const vector<array<i64, 2>> &lines) : t(-1000000000, 1000000001, (int)lines.size() + 200000) {
    for (auto &e : lines) t.add_line(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64 p) { return t.query(p); }
};
