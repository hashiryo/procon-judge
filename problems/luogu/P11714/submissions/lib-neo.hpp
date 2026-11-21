#pragma once
// NeoLibrary の sps::strongly_connected_digraph (neo/algo/count/graph/graph.hpp)。3^n の和を AVX2 で求め (法が奇数の ZMod)、
// f = -log(1 - g) を sps::log で出す。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/count/graph/graph.hpp"
struct Solver {
 using Z= ZMod<1000000007>;
 int n;
 sps::Edges es;
 u32 ans= 0;
 Solver(int n_, const vector<pair<int, int>>& es_): n(n_), es(es_) {}
 void run() { ans= sps::strongly_connected_digraph<Z>(n, es).back().val(); }
 u32 answer() const { return ans; }
};
