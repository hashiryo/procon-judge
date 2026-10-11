#pragma once
// 一般グラフの最大マッチング。二部の二重被覆の最大マッチングから始め、交互森で仕上げる。
//
// 二重被覆は、頂点 v を左の v と右の v に分け、辺 (u, v) を (左 u, 右 v) と (左 v, 右 u) の 2 本にした二部グラフ。その
// 最大マッチングを NeoLibrary の BipartiteMatching (push-relabel) で求めると、各頂点は出る相手と入る相手を高々 1 つずつ
// 持つので、もとのグラフの互いに素な道と閉路に分かれる (大きさの半分が最大の分数マッチング)。道と偶閉路は 1 つおきの
// 辺で頂点を全部 (道の長さが奇なら 1 つを除いて) 組ませ、奇閉路は 1 頂点だけ空ける。そこから gm_forest の交互森
// (RELABEL) で最大にする。二部に近いグラフでは奇閉路が少ないので、長い増加路をたどる探索がほとんど要らない。
#include "_shared/flow/gm_forest.hpp"
#include "neo/graph/BipartiteMatching.hpp"
namespace gm_dcover {
inline std::vector<int> general_matching(int n, const std::vector<std::array<int, 2>>& e) {
 std::vector<std::array<int, 2>> be;
 be.reserve(2 * e.size());
 for(auto [u, v]: e) be.push_back({u, v}), be.push_back({v, u});
 std::vector<int> out(n, -1), in(n, -1);
 {
  BipartiteMatching bm(n, n, be);
  for(int u= 0; u < n; ++u)
   if(const int w= bm.mate(u); w >= 0) out[u]= w - n, in[w - n]= u;
 }
 std::vector<int> mate(n, -1);
 std::vector<char> seen(n);
 // x から出る向きにたどり、隣り合う 2 頂点ずつ組ませる。
 auto walk= [&](int x) {
  while(x >= 0 && !seen[x]) {
   seen[x]= 1;
   const int y= out[x];
   if(y < 0 || seen[y]) break;
   seen[y]= 1, mate[x]= y, mate[y]= x, x= out[y];
  }
 };
 for(int v= 0; v < n; ++v)
  if(in[v] < 0) walk(v);  // 道の始点
 for(int v= 0; v < n; ++v) walk(v);  // 残りは閉路
 std::vector<int> off, adj;
 gm_gabow::build_csr(n, e, off, adj);
 gm_forest::finish<true>(n, off, adj, mate);
 return mate;
}
}
