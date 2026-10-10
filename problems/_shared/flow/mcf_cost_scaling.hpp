#pragma once
// 最小費用の b-flow を cost scaling (Goldberg と Tarjan の push-relabel 版) で求める。費用を (n + 1) 倍して、ε を
// 費用の絶対値の最大から ALPHA ずつ割り、ε = 1 まで refine を回す。refine は、縮約費用 c + p_v - p_w が負の残余の
// 辺をすべて飽和させてから、余りのある頂点を FIFO で取り出し、縮約費用が負の残余の辺 (admissible) に流す。流せる辺が
// 無ければ p_v を下げて (relabel)、縮約費用の最小が -ε になるようにする。ε = 1 で終えた流れは、元の費用で 1 / (n + 1)
// 最適なので最適になる。返すポテンシャルは、(n + 1) で割った値から始めて、残余グラフで label-correcting をかけて
// 元の費用での相補性を満たすように直す。b-flow があるかは、始める前に超頂点の間の最大流 (NeoLibrary の MaxFlow) で
// 確かめる (無い入力では refine が止まらない)。下限は先に流し、自己ループは費用が負なら上限、それ以外は下限まで流して
// 残余グラフに入れない。
#include <algorithm>
#include <array>
#include <cstdlib>
#include <queue>
#include <utility>
#include <vector>
#include "neo/flow/MaxFlow.hpp"
namespace mcf_cost_scaling {
using i64= long long;
struct Result {
 bool ok= false;
 __int128 cost= 0;
 std::vector<i64> pot, flow;
};
// PU なら global price update を、LA なら push look-ahead を使う。
// - global price update: 余りの負の頂点から残余の辺を逆にたどる最短路 (辺の長さは floor(縮約費用 / ε) + 1) を Dijkstra で
//   求め、p_v を ε d(v) だけ下げる。余りのある頂点がどれも負の頂点へ admissible な道を持つようになる。余りのある頂点を
//   すべて取り出したら打ち切り、残りの頂点は最後に取り出した距離にする。refine の初めと、relabel が n 回たまるごとにかける。
// - push look-ahead: 余りの負でない頂点 w へ流す前に、w に admissible な辺があるかを見て、無ければ先に w を relabel する。
template <int ALPHA= 16, bool PU= false, bool LA= false> Result b_flow(int n, const std::vector<i64>& b, const std::vector<std::array<i64, 5>>& edges) {
 const int m= edges.size();
 Result res;
 std::vector<i64> ex(b.begin(), b.end());
 for(auto& [s, t, l, u, c]: edges) ex[s]-= l, ex[t]+= l;
 {
  i64 sum= 0, need= 0;
  for(int v= 0; v < n; ++v) sum+= ex[v], need+= ex[v] > 0 ? ex[v] : 0;
  if(sum != 0) return res;
  MaxFlow<i64> mf(n + 2);
  for(auto& [s, t, l, u, c]: edges)
   if(s != t && u > l) mf.add_edge((int)s, (int)t, u - l);
  for(int v= 0; v < n; ++v)
   if(ex[v] > 0) mf.add_edge(n, v, ex[v]);
   else if(ex[v] < 0) mf.add_edge(v, n + 1, -ex[v]);
  if(need > 0 && mf.flow(n, n + 1) != need) return res;
 }
 // CSR の残余グラフ。辺 i の行きの弧は pos[i]、帰りの弧は rev[pos[i]]。
 std::vector<int> st(n + 1), pos(m, -1);
 for(auto& [s, t, l, u, c]: edges)
  if(s != t) ++st[s + 1], ++st[t + 1];
 for(int v= 0; v < n; ++v) st[v + 1]+= st[v];
 const int A= st[n];
 std::vector<int> to(A), rev(A);
 std::vector<i64> cap(A), cost(A);
 const i64 S= n + 1;
 i64 eps= 1;
 {
  std::vector<int> p(st.begin(), st.end() - 1);
  for(int i= 0; i < m; ++i) {
   const auto& [s, t, l, u, c]= edges[i];
   if(s == t) continue;
   const int a= p[s]++, r= p[t]++;
   to[a]= (int)t, rev[a]= r, cap[a]= u - l, cost[a]= c * S;
   to[r]= (int)s, rev[r]= a, cap[r]= 0, cost[r]= -c * S;
   pos[i]= a;
   eps= std::max(eps, std::abs(c) * S);
  }
 }
 std::vector<i64> p(n, 0);
 std::vector<int> cur(n), q(n);
 std::vector<char> inq(n);
 std::vector<i64> dist(n);
 std::vector<char> done(n);
 long long relabels= 0;
 // relabel: 残余の辺の縮約費用の最小が -ε になるように p_v を下げる。残余の辺が無ければ何もせず false を返す。
 auto relabel= [&](int v) {
  i64 best= -(i64(1) << 62);
  bool any= false;
  for(int j= st[v]; j < st[v + 1]; ++j)
   if(cap[j] > 0) best= std::max(best, p[to[j]] - cost[j]), any= true;
  if(!any) return false;
  p[v]= best - eps, cur[v]= st[v], ++relabels;
  return true;
 };
 auto price_update= [&] {
  using P= std::pair<i64, int>;
  std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
  constexpr i64 INF= i64(1) << 62;
  int left= 0;
  for(int v= 0; v < n; ++v) {
   dist[v]= INF, done[v]= 0, left+= ex[v] > 0;
   if(ex[v] < 0) dist[v]= 0, pq.push({0, v});
  }
  i64 last= 0;
  while(!pq.empty() && left > 0) {
   const auto [d, w]= pq.top();
   pq.pop();
   if(done[w] || d != dist[w]) continue;
   done[w]= 1, last= d, left-= ex[w] > 0;
   for(int k= st[w]; k < st[w + 1]; ++k) {
    const int v= to[k], r= rev[k];  // r は v から w への弧
    if(done[v] || cap[r] == 0) continue;
    const i64 rc= cost[r] + p[v] - p[w];
    const i64 nd= d + (rc >= 0 ? rc / eps : -((-rc + eps - 1) / eps)) + 1;
    if(nd < dist[v]) dist[v]= nd, pq.push({nd, v});
   }
  }
  for(int v= 0; v < n; ++v) p[v]-= eps * (done[v] ? dist[v] : last), cur[v]= st[v];
 };
 // 費用がすべて 0 でも、余りを流すために refine を 1 回は回す。
 do {
  eps= std::max<i64>(1, eps / ALPHA);
  // 縮約費用が負の残余の辺を飽和させる。
  for(int v= 0; v < n; ++v)
   for(int k= st[v]; k < st[v + 1]; ++k)
    if(cap[k] > 0 && cost[k] + p[v] - p[to[k]] < 0) {
     const i64 d= cap[k];
     cap[k]= 0, cap[rev[k]]+= d, ex[v]-= d, ex[to[k]]+= d;
    }
  for(int v= 0; v < n; ++v) cur[v]= st[v];
  if constexpr(PU) price_update(), relabels= 0;
  // 余りのある頂点を、長さ n の輪の FIFO で回す。
  int qh= 0, qn= 0;
  for(int v= 0; v < n; ++v) {
   inq[v]= ex[v] > 0;
   if(inq[v]) q[(qh + qn++) % n]= v;
  }
  while(qn) {
   const int v= q[qh];
   qh= qh + 1 == n ? 0 : qh + 1, --qn, inq[v]= 0;
   while(ex[v] > 0) {
    int& k= cur[v];
    if(k == st[v + 1]) {
     relabel(v);
     if constexpr(PU)
      if(relabels >= n) price_update(), relabels= 0;
     continue;
    }
    const int w= to[k];
    if(cap[k] > 0 && cost[k] + p[v] - p[w] < 0) {
     if constexpr(LA) {
      if(ex[w] >= 0) {
       // w に admissible な辺が無ければ、流す前に w を relabel する (v から w への辺は admissible でなくなりうる)。
       int& kw= cur[w];
       while(kw < st[w + 1] && !(cap[kw] > 0 && cost[kw] + p[w] - p[to[kw]] < 0)) ++kw;
       if(kw == st[w + 1] && relabel(w)) continue;
      }
     }
     const i64 d= std::min(ex[v], cap[k]);
     cap[k]-= d, cap[rev[k]]+= d, ex[v]-= d, ex[w]+= d;
     if(ex[w] > 0 && !inq[w]) inq[w]= 1, q[(qh + qn++) % n]= w;
     if(cap[k] == 0) ++k;
    } else ++k;
   }
  }
 } while(eps > 1);
 // ポテンシャルを元の費用に直す。floor(p / S) は相補性を高々 1 しか崩さないので、残余の辺で label-correcting をかける。
 std::vector<i64> pi(n);
 for(int v= 0; v < n; ++v) pi[v]= p[v] >= 0 ? p[v] / S : -((-p[v] + S - 1) / S);
 {
  int qh= 0, qn= 0;
  for(int v= 0; v < n; ++v) q[qn++]= v, inq[v]= 1;
  while(qn) {
   const int v= q[qh];
   qh= qh + 1 == n ? 0 : qh + 1, --qn, inq[v]= 0;
   for(int k= st[v]; k < st[v + 1]; ++k)
    if(cap[k] > 0 && pi[v] + cost[k] / S < pi[to[k]]) {
     pi[to[k]]= pi[v] + cost[k] / S;
     if(!inq[to[k]]) inq[to[k]]= 1, q[(qh + qn++) % n]= to[k];
    }
  }
 }
 res.ok= true;
 res.pot= pi;
 res.flow.resize(m);
 for(int i= 0; i < m; ++i) {
  const auto& [s, t, l, u, c]= edges[i];
  res.flow[i]= pos[i] < 0 ? (c < 0 ? u : l) : l + cap[rev[pos[i]]];
  res.cost+= (__int128)res.flow[i] * c;
 }
 return res;
}
}
