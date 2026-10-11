#pragma once
// NeoLibrary の biconnected_components (neo/graph/biconnected_components.hpp)。この問題で書き比べた tarjan_crec_aos_d8pf_hp を写した
// もので、作業領域が 2 MB 未満なら huge page を頼まない分と、成分の頂点の列 (start と vertex) を run() の中で作る分が違う。
#include "pj.hpp"
#include "neo/graph/biconnected_components.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 BiconnectedComponents r;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() { r= biconnected_components(n, es); }
 vector<vector<int>> answer() const {
  vector<vector<int>> ret(r.count);
  for(int k= 0; k < r.count; ++k) ret[k].assign(r.vertex.begin() + r.start[k], r.vertex.begin() + r.start[k + 1]);
  return ret;
 }
};
