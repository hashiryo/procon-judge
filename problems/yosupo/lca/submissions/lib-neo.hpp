#pragma once
#include "pj.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition を親の列から作り、lca は鎖を上がって求める (O(log N))。
struct Solver {
  HeavyLightDecomposition h;
  Solver(int, const vector<int> &par) : h(par) {}
  int lca(int u, int v) const { return h.lca(u, v); }
};
