#pragma once
#include "pj.hpp"
#include "neo/graph/LCA.hpp"

// NeoLibrary の HeavyLightDecomposition を親の列から作り、LCA (行きがけ順の区間最小、O(1)) で答える。
struct Solver {
  LCA l;
  Solver(int, const vector<int> &par) : l(HeavyLightDecomposition(par)) {}
  int lca(int u, int v) const { return l(u, v); }
};
