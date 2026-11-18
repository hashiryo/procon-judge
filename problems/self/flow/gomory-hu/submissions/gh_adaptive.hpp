#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp) を、網ごとに Dinic か push-relabel かを選ぶ形
// (_shared/gomory_hu/adaptive.hpp) で流す。Dinic の段の数の移動平均が 4.5 を超えている間は push-relabel で流し、
// 8 回に 1 回は Dinic で流して平均を測り直す。
#include "pj.hpp"
#include "_shared/gomory_hu/adaptive.hpp"
#include "_shared/gomory_hu/gusfield.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    gomory_hu_shared::AdaptiveEngine<> e(n, edges);
    res = gomory_hu_shared::tree_edges(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
