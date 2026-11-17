#pragma once
// 診断用。lib-neo-lean の最初のマッチングを、Karp-Sipser から貪欲 (左の頂点を番号の順に見て空いている最初の隣と組ませる) に
// 替えたもの (bm_lean.hpp の BipartiteMatching<false, true>)。lib-neo-lean との差が Karp-Sipser の分になる。比べ終えたら消す。
#include <memory>
#include "bm_lean.hpp"
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 std::unique_ptr<bm_lean::BipartiteMatching<false, true>> bm;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { bm= std::make_unique<bm_lean::BipartiteMatching<false, true>>(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  ret.reserve(bm->size());
  for(int a= 0; a < L; ++a)
   if(const int b= bm->mate(a); b >= 0) ret.push_back({a, b - L});
  return ret;
 }
};
