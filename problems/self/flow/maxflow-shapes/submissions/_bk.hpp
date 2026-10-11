#pragma once
// self-flow-maxflow-shapes 用に procon-judge の scratch/flow/mf_bk.hpp を写し、辺に逆向きの容量を持たせたもの。
// 最大流を Boykov-Kolmogorov の方法で求める。s を根にする木 S と t を根にする木 T を、残余の辺で両側から育てる。S の頂点から
// T の頂点への残余の辺が見つかれば、両方の木の道をつないだ増加路に流す。流して容量が 0 になった木の辺の子は親を失う
// (孤児)。孤児は、同じ木の頂点で、根まで孤児を通らずにたどれるもののうち、根からの距離が最小のものを新しい親に選ぶ。
// 見つからなければ木から外し、その子を孤児にして、自分へ残余の辺を持つ同じ木の頂点を育て直す候補に戻す。親が根まで
// たどれるかは、時刻の印 ts と根からの距離 dist で、同じ時刻に確かめた頂点で打ち切る。育てる頂点は FIFO で回す。
// 画像の領域分割のような格子のグラフで速いことで知られ、長い増加路が多いグラフでは孤児が多く出て遅くなる。
#include <algorithm>
#include <array>
#include <vector>
namespace mf_bk {
inline long long max_flow(int n, int s, int t, const std::vector<std::array<int, 4>>& e) {
 using i64= long long;
 const int m= e.size();
 std::vector<int> st(n + 1);
 for(auto& [u, v, c, rc]: e) ++st[u + 1], ++st[v + 1];
 for(int v= 0; v < n; ++v) st[v + 1]+= st[v];
 std::vector<int> to(2 * m), rev(2 * m), from(2 * m);
 std::vector<i64> cap(2 * m);
 {
  std::vector<int> p(st.begin(), st.end() - 1);
  for(auto& [u, v, c, rc]: e) {
   const int a= p[u]++, b= p[v]++;
   to[a]= v, from[a]= u, rev[a]= b, cap[a]= c;
   to[b]= u, from[b]= v, rev[b]= a, cap[b]= rc;
  }
 }
 // tree: 0 は木の外、1 は S、2 は T。pa は親との辺 (S では親から自分への弧、T では自分から親への弧)。根は TERM、孤児は ORPH。
 constexpr int TERM= -2, ORPH= -3, NONE= -1;
 std::vector<char> tree(n), inq(n);
 std::vector<int> pa(n, NONE), ts(n, 0), dist(n, 0), q, orphans;
 q.reserve(n);
 int qh= 0, now= 0;
 auto activate= [&](int v) {
  if(!inq[v]) inq[v]= 1, q.push_back(v);
 };
 tree[s]= 1, pa[s]= TERM, tree[t]= 2, pa[t]= TERM;
 activate(s), activate(t);
 i64 flow= 0;
 for(;;) {
  // 育てる。S から T への残余の辺 meet を探す。
  int meet= -1;
  while(qh < (int)q.size()) {
   const int p= q[qh];
   if(!tree[p]) {
    inq[p]= 0, ++qh;
    continue;
   }
   if(tree[p] == 1) {
    for(int a= st[p]; a < st[p + 1]; ++a) {
     if(cap[a] == 0) continue;
     const int v= to[a];
     if(!tree[v]) tree[v]= 1, pa[v]= a, ts[v]= ts[p], dist[v]= dist[p] + 1, activate(v);
     else if(tree[v] == 2) {
      meet= a;
      break;
     }
    }
   } else {
    for(int a= st[p]; a < st[p + 1]; ++a) {
     const int b= rev[a];  // to[a] から p への弧
     if(cap[b] == 0) continue;
     const int v= to[a];
     if(!tree[v]) tree[v]= 2, pa[v]= b, ts[v]= ts[p], dist[v]= dist[p] + 1, activate(v);
     else if(tree[v] == 1) {
      meet= b;
      break;
     }
    }
   }
   if(meet >= 0) break;
   inq[p]= 0, ++qh;
  }
  if(meet < 0) break;
  // 流す。
  i64 d= cap[meet];
  for(int v= from[meet]; pa[v] != TERM; v= from[pa[v]]) d= std::min(d, cap[pa[v]]);
  for(int v= to[meet]; pa[v] != TERM; v= to[pa[v]]) d= std::min(d, cap[pa[v]]);
  cap[meet]-= d, cap[rev[meet]]+= d;
  for(int v= from[meet]; pa[v] != TERM;) {
   const int a= pa[v];
   cap[a]-= d, cap[rev[a]]+= d;
   const int up= from[a];
   if(cap[a] == 0) pa[v]= ORPH, orphans.push_back(v);
   v= up;
  }
  for(int v= to[meet]; pa[v] != TERM;) {
   const int a= pa[v];
   cap[a]-= d, cap[rev[a]]+= d;
   const int up= to[a];
   if(cap[a] == 0) pa[v]= ORPH, orphans.push_back(v);
   v= up;
  }
  flow+= d;
  // 孤児を付け直す。
  ++now;
  while(!orphans.empty()) {
   const int v= orphans.back();
   orphans.pop_back();
   const char tv= tree[v];
   int best= NONE, bestd= 1 << 30;
   for(int k= st[v]; k < st[v + 1]; ++k) {
    // 候補の親 u と、親から v への (S) / v から親への (T) 弧
    const int u= to[k], a= tv == 1 ? rev[k] : k;
    if(tree[u] != tv || cap[a] == 0) continue;
    // u から根までたどれるか。
    int d0= 0, w= u;
    for(;; ++d0) {
     if(ts[w] == now) {
      d0+= dist[w];
      break;
     }
     const int pw= pa[w];
     if(pw == TERM) {
      ts[w]= now, dist[w]= 0;
      break;
     }
     if(pw == ORPH) {
      d0= 1 << 30;
      break;
     }
     w= tv == 1 ? from[pw] : to[pw];
    }
    if(d0 >= (1 << 30)) continue;
    if(d0 < bestd) best= a, bestd= d0;
    // たどった頂点に時刻と距離を付ける。
    for(int x= u, dd= d0; ts[x] != now; x= tv == 1 ? from[pa[x]] : to[pa[x]]) ts[x]= now, dist[x]= dd--;
   }
   if(best != NONE) {
    pa[v]= best, ts[v]= now, dist[v]= bestd + 1;
    continue;
   }
   // 親が見つからない。木から外し、子を孤児に、自分へ残余の辺を持つ同じ木の頂点を育て直す候補にする。
   tree[v]= 0, pa[v]= NONE;
   for(int k= st[v]; k < st[v + 1]; ++k) {
    const int u= to[k];
    if(tree[u] != tv) continue;
    const int a= tv == 1 ? rev[k] : k;  // u から v (S) / v から u (T)
    if(cap[a] > 0) activate(u);
    const int pu= pa[u];
    if(pu >= 0 && (tv == 1 ? from[pu] : to[pu]) == v) pa[u]= ORPH, orphans.push_back(u);
   }
  }
 }
 return flow;
}
}
