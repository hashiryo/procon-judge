#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp) を、網を 1 回だけ組む Dinic (_shared/gomory_hu/dinic.hpp) で流す形。
// DinicEngine の DEG_CUT は false (立てると、流量が s か t の重み付きの次数に届いた時点で止め、最後の BFS を省く)。
#include "pj.hpp"
#include "_shared/gomory_hu/dinic.hpp"
#include "_shared/gomory_hu/gusfield.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    gomory_hu_shared::DinicEngine<false> e(n, edges);
    res = gomory_hu_shared::tree_edges(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
