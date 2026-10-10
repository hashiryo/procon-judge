#pragma once
// Dinic。s からの BFS で層を作り (t に届いた時点で止める)、層を 1 つずつ上る辺だけを辿る DFS で流す。DFS は
// 1 回の呼び出しで流せるだけ流し (multi-augment)、頂点ごとに次に見る辺の位置を持つ。SCALING が true なら容量の
// スケーリングを入れる。Δ を容量の最大以下の 2 の冪から始めて半分ずつにし、各 Δ で、残余容量が Δ 以上の辺だけを
// 使う Dinic を流せなくなるまで回す。辺は CSR に並べ、容量は int (残余容量は元の容量を超えない)、流量は i64 で
// 持つ。自己ループは捨てる。
#include <algorithm>
#include <array>
#include <limits>
#include <vector>
namespace mf_dinic {
using i64= long long;
struct E {
 int to, rev, cap;
};
struct Impl {
 int n, s, t, delta;
 std::vector<int> st, lv, it, q;
 std::vector<E> es;
 i64 dfs(int u, i64 f) {
  if(u == t) return f;
  i64 pushed= 0;
  for(int &k= it[u], ed= st[u + 1]; k < ed; ++k)
   if(E& e= es[k]; e.cap >= delta && lv[e.to] == lv[u] + 1) {
    const i64 d= dfs(e.to, std::min<i64>(f - pushed, e.cap));
    if(d > 0) {
     e.cap-= d, es[e.rev].cap+= d, pushed+= d;
     if(pushed == f) return pushed;
    }
   }
  return pushed;
 }
 bool bfs() {
  std::fill(lv.begin(), lv.end(), -1);
  int qh= 0, qt= 0;
  lv[s]= 0, q[qt++]= s;
  while(qh < qt && lv[t] < 0) {
   const int u= q[qh++];
   for(int k= st[u]; k < st[u + 1]; ++k)
    if(const E& e= es[k]; e.cap >= delta && lv[e.to] < 0) lv[e.to]= lv[u] + 1, q[qt++]= e.to;
  }
  return lv[t] >= 0;
 }
 i64 blocking_flows() {
  i64 flow= 0;
  while(bfs()) {
   std::copy(st.begin(), st.end() - 1, it.begin());
   flow+= dfs(s, std::numeric_limits<i64>::max());
  }
  return flow;
 }
};
// edges は {u, v, 容量}。容量は 0 以上。
template <bool SCALING> i64 max_flow(int n, int s, int t, const std::vector<std::array<int, 3>>& edges) {
 Impl g{n, s, t, 1, std::vector<int>(n + 1), std::vector<int>(n), std::vector<int>(n), std::vector<int>(n), {}};
 auto& st= g.st;
 for(auto& e: edges)
  if(e[0] != e[1]) ++st[e[0] + 1], ++st[e[1] + 1];
 for(int i= 0; i < n; ++i) st[i + 1]+= st[i];
 g.es.resize(st[n]);
 {
  std::vector<int> pos(st.begin(), st.end() - 1);
  for(auto& e: edges)
   if(e[0] != e[1]) {
    const int a= pos[e[0]]++, b= pos[e[1]]++;
    g.es[a]= {e[1], b, e[2]}, g.es[b]= {e[0], a, 0};
   }
 }
 if constexpr(!SCALING) return g.blocking_flows();
 else {
  int mx= 1;
  for(auto& e: edges) mx= std::max(mx, e[2]);
  i64 flow= 0;
  for(g.delta= 1 << (31 - __builtin_clz(mx)); g.delta; g.delta>>= 1) flow+= g.blocking_flows();
  return flow;
 }
}
}
