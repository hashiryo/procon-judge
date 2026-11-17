#pragma once
// 診断用。bm_lean.hpp の BipartiteMatching<true, true> で解く。NeoLibrary の BipartiteMatching から、辺の列を写さない形に
// 変えたもの (huge page はこの大きさでは効かない)。lib-neo との差が辺の列を写す分になる。比べ終えたら消す。
#include <memory>
#include "bm_lean.hpp"
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 std::unique_ptr<bm_lean::BipartiteMatching<true, true>> bm;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { bm= std::make_unique<bm_lean::BipartiteMatching<true, true>>(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  ret.reserve(bm->size());
  for(int a= 0; a < L; ++a)
   if(const int b= bm->mate(a); b >= 0) ret.push_back({a, b - L});
  return ret;
 }
};
