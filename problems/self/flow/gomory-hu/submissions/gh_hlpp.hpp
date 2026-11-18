#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp) を、網を 1 回だけ組む highest label の push-relabel
// (_shared/gomory_hu/hlpp.hpp) で流す形。前流の段だけを回し、最小カットは t に届かない頂点の集合で取る。
#include "pj.hpp"
#include "_shared/gomory_hu/gusfield.hpp"
#include "_shared/gomory_hu/hlpp.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    gomory_hu_shared::HlppEngine e(n, edges);
    res = gomory_hu_shared::tree_edges(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
