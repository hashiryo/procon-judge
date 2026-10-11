#pragma once
// NeoLibrary の two_edge_connected_components (neo/graph/two_edge_connected_components.hpp)。この問題で書き比べた gabow_crec_aos_d8pf_hp を
// 写したもので、作業領域が 2 MB 未満なら huge page を頼まない分と、成分の番号を 0 から詰めた列を run() の中で作る分が違う。
#include "pj.hpp"
#include "neo/graph/two_edge_connected_components.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 TwoEdgeConnectedComponents r;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() { r= two_edge_connected_components(n, es); }
 int comp(int v) const { return r.comp[v]; }
};
