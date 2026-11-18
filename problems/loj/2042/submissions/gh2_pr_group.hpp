#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp の gusfield_grouped) を、_shared/gomory_hu/engine2.hpp の MODE 1
// (push-relabel だけ) で、TOUCH と CACHE = 2 を立てて流す形。gh2_pr_cache2 と同じ流し方で、頂点を処理する順を、同じ親を持つ
// 未処理の頂点が続くように選ぶ (作る木は gh2_pr と違いうるが、どれも Gomory-Hu 木)。
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
    res = gomory_hu_shared::tree_edges<decltype(e), true>(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
