#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp) を、_shared/gomory_hu/engine2.hpp の MODE 1
// (push-relabel だけ) で、TOUCH と CACHE = 2 を立てて流す形。最初の高さの BFS を、余りのある頂点に距離が付くところまでだけ進め、
// t が前の flow と同じなら続きから進める。作る木は gh2_pr と同じ。
#include "pj.hpp"
#include "_shared/gomory_hu/engine2.hpp"
#include "_shared/gomory_hu/gusfield.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    gomory_hu_shared::Engine2<1, true, 16, true, 2> e(n, edges);
    res = gomory_hu_shared::tree_edges<decltype(e), false>(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
