#pragma once
#include "pj.hpp"
#include "neo/algo/compute/permutation/permutation_tree.hpp"

// NeoLibrary の permutation_tree を使う提出。構築順のまま出す (checker は番号を問わない)。
struct Solver {
  using Node = PermutationTreeNode;
  int n;
  vector<int> p;
  vector<PermutationTreeNode> nodes;
  Solver(int n_, const vector<int> &p_) : n(n_), p(p_) {}
  void run() { nodes = permutation_tree(p); }
  const vector<PermutationTreeNode> &answer() const { return nodes; }
};
