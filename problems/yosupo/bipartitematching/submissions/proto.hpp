#pragma once
// _shared/flow/proto_bipartite.hpp の試作の API で書いたもの。マッチングは pr_ks_half_pack と同じ方式で、辺の列を写して
// 持つ分と、相手を入口の番号で引く分だけが違う。
#include <memory>
#include "_shared/flow/proto_bipartite.hpp"
struct Solver {
 int L, R;
 const vector<array<int, 2>>& e;
 std::unique_ptr<proto::BipartiteMatching> bm;
 Solver(int l, int r, const vector<array<int, 2>>& edges): L(l), R(r), e(edges) {}
 void run() { bm= std::make_unique<proto::BipartiteMatching>(L, R, e); }
 vector<array<int, 2>> answer() const {
  vector<array<int, 2>> ret;
  ret.reserve(bm->size());
  for(int a= 0; a < L; ++a)
   if(const int b= bm->mate(a); b >= 0) ret.push_back({a, b - L});
  return ret;
 }
};
