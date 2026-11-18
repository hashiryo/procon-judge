#pragma once
// Gusfield の方法 (_shared/gomory_hu/gusfield.hpp) を、_shared/gomory_hu/engine2.hpp の MODE 1 で流す形。push-relabel だけで流す (前流の段のあと、カットが {s} でも t 以外の全部でもないときだけ余りを s へ戻す)。
// どの MODE も同じ最小カット (流量が s か t の重み付きの次数に等しければ {s} か t 以外の全部、ほかは s の側の最小の集合)
// を返すので、作る木は MODE によらない。
#include "pj.hpp"
#include "_shared/gomory_hu/engine2.hpp"
#include "_shared/gomory_hu/gusfield.hpp"
struct Solver {
  int n;
  const vector<array<i64, 3>> &edges;
  vector<array<i64, 3>> res;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    gomory_hu_shared::Engine2<1> e(n, edges);
    res = gomory_hu_shared::tree_edges(n, e);
  }
  const vector<array<i64, 3>> &tree() const { return res; }
};
