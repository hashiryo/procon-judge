#pragma once
#include "_tree_gen.hpp"

// p_i < i を使わない一般の HLD。子の数を数えて葉から剥がした順で作る (中身は _tree_gen.hpp)。問い合わせは hld_hp_b と同じ。
struct Solver {
  lca_gen::Tree<0> t;
  Solver(int n, const vector<int> &par) : t(n, par) {}
  int lca(int u, int v) const { return t.lca(u, v); }
};
