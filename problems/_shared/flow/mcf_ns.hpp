#pragma once
// 最小費用の b-flow をネットワーク単体法で求める。今の Library の NetworkSimplex (block search、子のリストで部分木を
// たどる) と同じ手順で、辺と頂点の持ち方を配列ごとに分けたもの (SoA)。
//
// 入る辺の選び方 (block search) は、辺を block の大きさずつ見て、縮約費用で破れている度合いが最大の辺を選ぶ。走査では
// 状態、費用、両端と、両端のポテンシャルしか読まないので、それらを別々の配列に置き、辺 1 本あたり 17 バイトほどで済ませる
// (Library は 1 本の構造体に 56 バイトほど持つ)。block の大きさは ceil(sqrt(m + n)) の BF_NUM / BF_DEN 倍。
// 根 n と、各頂点から根へ (または根から) の人工の辺 (費用は費用の絶対値の和 + 1) から始める。
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>
namespace mcf_ns {
using i64= long long;
struct Result {
 bool ok= false;
 __int128 cost= 0;
 std::vector<i64> pot, flow;
};
template <int BF_NUM= 1, int BF_DEN= 1> Result b_flow(int n, const std::vector<i64>& b, const std::vector<std::array<i64, 5>>& edges) {
 Result res;
 const int m= edges.size(), M= m + n;
 {
  i64 sum= 0;
  for(int v= 0; v < n; ++v) sum+= b[v];
  if(sum != 0) return res;
 }
 // 頂点 (根 n を含む)。
 std::vector<int> par(n + 1), pred(n + 1);
 std::vector<i64> pi(n + 1), sup(n + 1);
 for(int v= 0; v < n; ++v) sup[v]= b[v];
 // 辺 (人工の辺 n 本を後ろに足す)。state は 1 が下限、-1 が上限、0 が木。
 std::vector<int> eu(M), ev(M);
 std::vector<i64> up(M), flow(M), cost(M);
 std::vector<int8_t> state(M, 1);
 i64 inf_cost= 1;
 for(int e= 0; e < m; ++e) {
  const auto& [s, t, l, u, c]= edges[e];
  eu[e]= (int)s, ev[e]= (int)t, up[e]= u - l, cost[e]= c;
  sup[s]-= l, sup[t]+= l;
  inf_cost+= c < 0 ? -c : c;
 }
 // 部分木をたどるための子のリスト。頂点 u の子は、番兵 u + n + 1 から nxt でつながる。
 std::vector<int> nxt(2 * n + 2), prv(2 * n + 2), bfs(n + 1);
 std::iota(nxt.begin() + n + 1, nxt.end(), n + 1);
 std::iota(prv.begin() + n + 1, prv.end(), n + 1);
 auto link= [&](int u, int v) { nxt[u]= v, prv[v]= u; };
 auto link3= [&](int u, int v, int w) { link(u, v), link(v, w); };
 par[n]= -1, pred[n]= -1, pi[n]= 0;
 for(int u= 0, e= m; u < n; ++u, ++e) {
  par[u]= n, pred[u]= e, link3(prv[n + n + 1], u, n + n + 1);
  state[e]= 0, cost[e]= inf_cost;
  if(sup[u] >= 0) pi[u]= -inf_cost, eu[e]= u, ev[e]= n, up[e]= flow[e]= sup[u];
  else pi[u]= inf_cost, eu[e]= n, ev[e]= u, up[e]= flow[e]= -sup[u];
 }
 auto opp_cost= [&](int e) { return cost[e] + pi[eu[e]] - pi[ev[e]]; };
 auto pivot= [&](int in_arc) {
  int u_in= eu[in_arc], v_in= ev[in_arc], u, e, a= u_in, bb= v_in;
  while(a != bb) a= par[a] == -1 ? v_in : par[a], bb= par[bb] == -1 ? u_in : par[bb];
  if(state[in_arc] == -1) std::swap(u_in, v_in);
  int lca= a, side= 0, u_out= -1, i= 0, S= 0;
  i64 delta= up[in_arc];
  for(u= u_in; u != lca && delta > 0; u= par[u]) {
   e= pred[u];
   const i64 d= u == ev[e] ? up[e] - flow[e] : flow[e];
   if(delta > d) delta= d, u_out= u, side= 1;
  }
  for(u= v_in; u != lca; u= par[u]) {
   e= pred[u];
   const i64 d= u == eu[e] ? up[e] - flow[e] : flow[e];
   if(delta >= d) delta= d, u_out= u, side= -1;
  }
  if(delta > 0) {
   flow[in_arc]+= delta*= state[in_arc];
   for(u= eu[in_arc]; u != lca; u= par[u]) e= pred[u], flow[e]+= u == eu[e] ? -delta : delta;
   for(u= ev[in_arc]; u != lca; u= par[u]) e= pred[u], flow[e]+= u == eu[e] ? delta : -delta;
  }
  if(side == 0) {
   state[in_arc]*= -1;
   return;
  }
  const int out_arc= pred[u_out];
  state[in_arc]= 0, state[out_arc]= flow[out_arc] ? -1 : 1;
  if(side == -1) std::swap(u_in, v_in);
  for(u= u_in; u != u_out; u= par[u]) bfs[S++]= u;
  for(i= S; i--;) {
   u= bfs[i];
   const int p= par[u];
   par[p]= u, pred[p]= pred[u], link(prv[p], nxt[p]), link3(prv[u + n + 1], p, u + n + 1);
  }
  link(prv[u_in], nxt[u_in]), link3(prv[v_in + n + 1], u_in, v_in + n + 1);
  par[u_in]= v_in, pred[u_in]= in_arc;
  const i64 pi_delta= u_in == eu[in_arc] ? -opp_cost(in_arc) : opp_cost(in_arc);
  for(i= 0, S= 1, bfs[0]= u_in; i < S; i++) {
   pi[u= bfs[i]]+= pi_delta;
   for(int v= nxt[u + n + 1]; v != u + n + 1; v= nxt[v]) bfs[S++]= v;
  }
 };
 const int block_size= std::max(1, std::max(int(std::ceil(std::sqrt((double)M))) * BF_NUM / BF_DEN, std::min(10, n + 1)));
 for(int e= 0, in_arc, cnt, seen;; pivot(in_arc)) {
  i64 minimum= 0;
  for(in_arc= -1, cnt= block_size, seen= M; seen--; e= e + 1 == M ? 0 : e + 1) {
   if(const i64 c= state[e] * opp_cost(e); minimum > c) minimum= c, in_arc= e;
   if(--cnt == 0) {
    if(minimum < 0) break;
    cnt= block_size;
   }
  }
  if(in_arc == -1) break;
 }
 for(int e= m; e < M; ++e)
  if(flow[e] != 0) return res;
 res.ok= true;
 res.pot.assign(pi.begin(), pi.begin() + n);
 res.flow.resize(m);
 for(int e= 0; e < m; ++e) res.flow[e]= flow[e] + edges[e][2], res.cost+= (__int128)res.flow[e] * edges[e][4];
 return res;
}
}
