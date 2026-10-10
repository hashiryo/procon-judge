#pragma once
// NeoLibrary の BipartiteMatching (neo/graph/BipartiteMatching.hpp) で書いたもの。マッチングは pr_ks_half_pack と同じ方式で、辺の列を写して
// 持つ分と、相手を入口の番号で引く分だけが違う。
#include <memory>
#include "neo/graph/BipartiteMatching.hpp"
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 std::unique_ptr<BipartiteMatching> bm;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { bm= std::make_unique<BipartiteMatching>(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  ret.reserve(bm->size());
  for(int a= 0; a < L; ++a)
   if(const int b= bm->mate(a); b >= 0) ret.push_back({a, b - L});
  return ret;
 }
};
