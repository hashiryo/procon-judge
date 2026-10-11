#pragma once
// 最小費用の b-flow を cost scaling で求める。mcf_cost_scaling.hpp の global price update を Dial のバケツにし、流し方に
// LEMON の partial augment を選べるようにしたもの。
//
// global price update は、余りの負の頂点から残余の辺を逆にたどる最短路 (辺の長さは floor(縮約費用 / ε) + 1、0 以上の
// 整数) を、距離ごとのバケツで求める。距離が n を超える頂点はたどらず、余りのある頂点をすべて取り出したら打ち切って、
// 残りの頂点は最後に取り出した距離にする。refine の初めと、relabel が n 回たまるごとにかける。
//
// MAXLEN が 0 なら、mcf_cost_scaling の PU と LA と同じ push で流す。正なら partial augment で流す。余りのある頂点から、
// admissible な辺 (縮約費用が負の残余の辺) をたどって道を伸ばし、余りの負の頂点に着くか、長さが MAXLEN になったら、
// 道に沿って流せるだけ流す (各辺で、手前の頂点の余りと容量の小さいほう)。行き詰まったら、その頂点を relabel して 1 歩
// 戻る。道が自分自身に戻ったら、そこで流す。
#include <algorithm>
#include <array>
#include <cstdlib>
#include <utility>
#include <vector>
#include "neo/flow/MaxFlow.hpp"
namespace mcf_cs2 {
using i64= long long;
struct Result {
 bool ok= false;
 __int128 cost= 0;
 std::vector<i64> pot, flow;
};
// PUF_NUM / PUF_DEN は global price update の間隔 (relabel が n PUF_NUM / PUF_DEN 回たまるごと)、RANK は距離の上限 (n の倍数)。
// ET が 0 でなければ早く打ち切る。ε が S / ET 以下の段を終えるたびに、ポテンシャルを S で割った値から残余グラフで
// label-correcting をかけ (関係を直す回数は A / 2 + n まで)、収まれば今の流れは元の費用で最適なので、そこで終える。費用を
// S 倍しているので、ε が S を下回った段で元の費用では 1 未満の最適性まで来ていて、手元のケースではどれも ε が S / 100 ほどの
// 段で最適になっていた。
template <int ALPHA= 16, int MAXLEN= 0, int PUF_NUM= 1, int PUF_DEN= 1, int RANK= 1, int ET= 0> Result b_flow(int n, const std::vector<i64>& b, const std::vector<std::array<i64, 5>>& edges) {
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
 std::vector<int> st(n + 1), pos(m, -1);
 for(auto& [s, t, l, u, c]: edges)
  if(s != t) ++st[s + 1], ++st[t + 1];
 for(int v= 0; v < n; ++v) st[v + 1]+= st[v];
 const int A= st[n];
 std::vector<int> to(A), rev(A), from(A);
 std::vector<i64> cap(A), cost(A);
 const i64 S= n + 1;
 i64 eps= 1;
 {
  std::vector<int> p(st.begin(), st.end() - 1);
  for(int i= 0; i < m; ++i) {
   const auto& [s, t, l, u, c]= edges[i];
   if(s == t) continue;
   const int a= p[s]++, r= p[t]++;
   to[a]= (int)t, from[a]= (int)s, rev[a]= r, cap[a]= u - l, cost[a]= c * S;
   to[r]= (int)s, from[r]= (int)t, rev[r]= a, cap[r]= 0, cost[r]= -c * S;
   pos[i]= a;
   eps= std::max(eps, std::abs(c) * S);
  }
 }
 std::vector<i64> p(n, 0);
 std::vector<int> cur(n), q(n), rank(n), bhead((size_t)n * RANK + 2), bnext, bnode;
 std::vector<char> inq(n), done(n), onpath(MAXLEN > 0 ? A : 0);
 long long relabels= 0;
 const long long pu_every= std::max<long long>(1, (long long)n * PUF_NUM / PUF_DEN);
 // extra が 0 以上なら、その弧も (容量が無くても) 残余の辺として扱う。partial augment で道の先を relabel するときに、
 // 来た辺の逆向きを入れて、来た辺が admissible でなくなるようにする (行き止まりの頂点で同じ辺を行き来しないため)。
 auto relabel= [&](int v, int extra= -1) {
  i64 best= -(i64(1) << 62);
  bool any= false;
  for(int j= st[v]; j < st[v + 1]; ++j)
   if(cap[j] > 0) best= std::max(best, p[to[j]] - cost[j]), any= true;
  if(extra >= 0) best= std::max(best, p[to[extra]] - cost[extra]), any= true;
  if(!any) return false;
  p[v]= best - eps, cur[v]= st[v], ++relabels;
  return true;
 };
 // Dial のバケツ。頂点は距離が下がるたびに新しいバケツへ積み (古い場所は残す)、取り出すときに距離が合うかを見る。
 auto price_update= [&] {
  const int cap_rank= n * RANK;
  std::fill(bhead.begin(), bhead.end(), -1);
  bnext.clear(), bnode.clear();
  auto push= [&](int r, int v) { bnode.push_back(v), bnext.push_back(bhead[r]), bhead[r]= (int)bnode.size() - 1; };
  int left= 0;
  for(int v= 0; v < n; ++v) {
   rank[v]= cap_rank + 1, done[v]= 0, left+= ex[v] > 0;
   if(ex[v] < 0) rank[v]= 0, push(0, v);
  }
  int last= 0;
  for(int r= 0; r <= cap_rank && left > 0; ++r) {
   while(bhead[r] >= 0 && left > 0) {
    const int id= bhead[r];
    bhead[r]= bnext[id];
    const int w= bnode[id];
    if(done[w] || rank[w] != r) continue;
    done[w]= 1, last= r, left-= ex[w] > 0;
    for(int k= st[w]; k < st[w + 1]; ++k) {
     const int v= to[k], ra= rev[k];  // ra は v から w への弧
     if(done[v] || cap[ra] == 0) continue;
     const i64 rc= cost[ra] + p[v] - p[w];
     const i64 nd= r + (rc >= 0 ? rc / eps : -((-rc + eps - 1) / eps)) + 1;
     if(nd < rank[v] && nd <= cap_rank) rank[v]= (int)nd, push((int)nd, v);
    }
   }
  }
  for(int v= 0; v < n; ++v) p[v]-= eps * (done[v] ? rank[v] : last), cur[v]= st[v];
  relabels= 0;
 };
 // ポテンシャルを S で割った値から、残余グラフで元の費用の label-correcting をかけて相補性を満たすように直す。関係を直す
 // 回数が budget (負なら無制限) を超えたら false を返す (負の閉路があり、まだ最適でない)。
 std::vector<i64> pi(n);
 bool settled= false;
 auto settle= [&](long long budget) {
  for(int v= 0; v < n; ++v) pi[v]= p[v] >= 0 ? p[v] / S : -((-p[v] + S - 1) / S);
  int qh= 0, qn= 0;
  for(int v= 0; v < n; ++v) q[qn++]= v, inq[v]= 1;
  long long relax= 0;
  while(qn) {
   const int v= q[qh];
   qh= qh + 1 == n ? 0 : qh + 1, --qn, inq[v]= 0;
   for(int k= st[v]; k < st[v + 1]; ++k)
    if(cap[k] > 0 && pi[v] + cost[k] / S < pi[to[k]]) {
     pi[to[k]]= pi[v] + cost[k] / S;
     if(!inq[to[k]]) inq[to[k]]= 1, q[(qh + qn++) % n]= to[k];
     if(++relax == budget) {
      std::fill(inq.begin(), inq.end(), 0);
      return false;
     }
    }
  }
  return settled= true;
 };
 std::vector<int> path;
 do {
  eps= std::max<i64>(1, eps / ALPHA);
  for(int v= 0; v < n; ++v)
   for(int k= st[v]; k < st[v + 1]; ++k)
    if(cap[k] > 0 && cost[k] + p[v] - p[to[k]] < 0) {
     const i64 d= cap[k];
     cap[k]= 0, cap[rev[k]]+= d, ex[v]-= d, ex[to[k]]+= d;
    }
  price_update();
  int qh= 0, qn= 0;
  for(int v= 0; v < n; ++v) {
   inq[v]= ex[v] > 0;
   if(inq[v]) q[(qh + qn++) % n]= v;
  }
  auto activate= [&](int w) {
   if(ex[w] > 0 && !inq[w]) inq[w]= 1, q[(qh + qn++) % n]= w;
  };
  while(qn) {
   const int v= q[qh];
   qh= qh + 1 == n ? 0 : qh + 1, --qn, inq[v]= 0;
   if constexpr(MAXLEN == 0) {
    while(ex[v] > 0) {
     int& k= cur[v];
     if(k == st[v + 1]) {
      relabel(v);
      if(relabels >= pu_every) price_update();
      continue;
     }
     const int w= to[k];
     if(cap[k] > 0 && cost[k] + p[v] - p[w] < 0) {
      if(ex[w] >= 0) {
       int& kw= cur[w];
       while(kw < st[w + 1] && !(cap[kw] > 0 && cost[kw] + p[w] - p[to[kw]] < 0)) ++kw;
       if(kw == st[w + 1] && relabel(w)) continue;
      }
      const i64 d= std::min(ex[v], cap[k]);
      cap[k]-= d, cap[rev[k]]+= d, ex[v]-= d, ex[w]+= d;
      activate(w);
      if(cap[k] == 0) ++k;
     } else ++k;
    }
   } else {
    while(ex[v] > 0) {
     // v から道を伸ばす。
     int tip= v;
     path.clear();
     while((int)path.size() < MAXLEN && ex[tip] >= 0) {
      int& k= cur[tip];
      while(k < st[tip + 1] && !(cap[k] > 0 && cost[k] + p[tip] - p[to[k]] < 0)) ++k;
      if(k < st[tip + 1]) {
       const int a= k;
       path.push_back(a);
       if(onpath[a]) break;  // 輪になった
       onpath[a]= 1, tip= to[a];
       continue;
      }
      relabel(tip, tip != v ? rev[path.back()] : -1);
      if(tip != v) {
       const int a= path.back();
       path.pop_back(), onpath[a]= 0, tip= from[a];
      }
     }
     // 道に沿って流す。
     for(const int a: path) {
      onpath[a]= 0;
      const int u= from[a], w= to[a];
      const i64 d= std::min(cap[a], ex[u]);
      cap[a]-= d, cap[rev[a]]+= d, ex[u]-= d, ex[w]+= d;
      if(w != v) activate(w);
     }
     if(relabels >= pu_every) price_update();
     if(path.empty()) break;  // 起きない (余りのある頂点には残余の辺がある)
    }
   }
  }
  if constexpr(ET > 0)
   if(eps > 1 && eps * ET <= S && settle(A / 2 + n)) break;
 } while(eps > 1);
 if(!settled) settle(-1);
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
