#pragma once
#include "_tree_gen.hpp"

// 一般の HLD (中身は _tree_gen.hpp) で、先に p_i < i (根が 0) かを確かめ、そうなら番号の大きい順をそのまま下から上の順に
// 使う。違えば gen_peel と同じく葉から剥がす。問い合わせは hld_hp_b と同じ。
struct Solver {
  lca_gen::Tree<2> t;
  Solver(int n, const vector<int> &par) : t(n, par) {}
  int lca(int u, int v) const { return t.lca(u, v); }
};
