#pragma once
// NeoLibrary の dominator_tree (neo/graph/dominator_tree.hpp)。この問題で書き比べた snca_halve_hp を写したもので、作業領域が 2 MB 未満なら
// huge page を頼まない分と、頂点ごとの直接支配点の列を run() の中で作る分が違う。
#include "pj.hpp"
#include "neo/graph/dominator_tree.hpp"
struct Solver {
 int n, s;
 const vector<array<int, 2>>& es;
 vector<int> p;
 Solver(int n, int s, const vector<array<int, 2>>& es): n(n), s(s), es(es) {}
 void run() { p= dominator_tree(n, s, es); }
 int parent(int v) const { return p[v]; }
};
