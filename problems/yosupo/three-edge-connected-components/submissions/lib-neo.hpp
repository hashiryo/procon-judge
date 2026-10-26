#pragma once
// NeoLibrary の three_edge_connected_components (neo/graph/three_edge_connected_components.hpp)。この問題で書き比べた tsin_chain_hp を写した
// もので、作業領域が 2 MB 未満なら huge page を頼まない分と、成分の番号を 0 から詰めた列を run() の中で作る分が違う。番号を振る走査は
// NeoLibrary 937f57f で分岐なしにした。
#include "pj.hpp"
#include "neo/graph/three_edge_connected_components.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 ThreeEdgeConnectedComponents r;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() { r= three_edge_connected_components(n, es); }
 int comp(int v) const { return r.comp[v]; }
};
