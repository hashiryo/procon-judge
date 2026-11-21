#pragma once
// NeoLibrary の bipartite_edge_coloring (neo/algo/optimize/matching/bipartite_edge_coloring.hpp)。中身は mult_pair と同じ。
#include "neo/algo/optimize/matching/bipartite_edge_coloring.hpp"
struct Solver {
 int l, r;
 const vector<array<int, 2>>& e;
 vector<int> c;
 Solver(int l, int r, const vector<array<int, 2>>& edges): l(l), r(r), e(edges) {}
 void run() { c= bipartite_edge_coloring(l, r, e); }
 const vector<int>& colors() const { return c; }
};
