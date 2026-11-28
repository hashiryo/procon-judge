#pragma once
#include "pj.hpp"
#include "neo/algo/compute/group/permutation_group_order.hpp"

// NeoLibrary の permutation_group_order を使う提出。
struct Solver {
  int n;
  vector<vector<int>> gen;
  unsigned res = 0;
  Solver(int n_, const vector<vector<int>> &g) : n(n_), gen(g) {}
  void run() { res = permutation_group_order(n, gen); }
  unsigned answer() const { return res; }
};
