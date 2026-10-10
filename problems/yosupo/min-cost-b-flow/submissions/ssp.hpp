#pragma once
// 逐次最短路 (primal-dual)。下限は先に流しておき、費用が負の辺は上限まで流しておく (残余グラフの辺の費用がすべて 0 以上
// になる)。そうして残った各頂点の過不足を、超頂点 S から湧き出し、超頂点 T へ吸い込む流れに直し、S から T への最短路を
// Dijkstra で探して流すことを繰り返す。ポテンシャルは ACL の min_cost_flow と同じく、Dijkstra で訪れた頂点 v だけを
// dist[T] - dist[v] だけ下げる。訪れなかった頂点も含めて、残余グラフのすべての辺で縮約費用 c + p_u - p_v が 0 以上に
// 保たれるので、そのまま相補性を満たすポテンシャルとして返せる。自己ループは、費用が負なら上限、それ以外は下限まで
// 流したまま残余グラフに入れない。
#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <vector>
namespace mcf_ssp {
using i64= long long;
struct E {
 int to, rev;
 i64 cap, cost;
};
struct Result {
 bool ok= false;
 __int128 cost= 0;
 std::vector<i64> pot, flow;
};
// edges は {s, t, 下限, 上限, 費用}。
inline Result b_flow(int n, const std::vector<i64>& b, const std::vector<std::array<i64, 5>>& edges) {
 const int m= edges.size(), S= n, T= n + 1, N= n + 2;
 std::vector<i64> bb(b.begin(), b.end());
 std::vector<std::vector<E>> g(N);
 std::vector<std::pair<int, int>> pos(m, {-1, -1});
 for(int i= 0; i < m; ++i) {
  const auto& [s, t, l, u, c]= edges[i];
  const i64 f0= c < 0 ? u : l, cap= u - l;
  bb[s]-= f0, bb[t]+= f0;
  if(s == t) continue;
  pos[i]= {(int)s, (int)g[s].size()};
  g[s].push_back({(int)t, (int)g[t].size(), c < 0 ? 0 : cap, c});
  g[t].push_back({(int)s, (int)g[s].size() - 1, c < 0 ? cap : 0, -c});
 }
 Result res;
 i64 sum= 0, need= 0;
 for(int v= 0; v < n; ++v) {
  sum+= bb[v];
  if(bb[v] > 0) {
   need+= bb[v];
   g[S].push_back({v, (int)g[v].size(), bb[v], 0});
   g[v].push_back({S, (int)g[S].size() - 1, 0, 0});
  } else if(bb[v] < 0) {
   g[v].push_back({T, (int)g[T].size(), -bb[v], 0});
   g[T].push_back({v, (int)g[v].size() - 1, 0, 0});
  }
 }
 if(sum != 0) return res;
 const i64 INF= std::numeric_limits<i64>::max() / 4;
 std::vector<i64> dual(N, 0), dist(N);
 std::vector<int> pv(N), pe(N);
 std::vector<char> vis(N);
 i64 flow= 0;
 using P= std::pair<i64, int>;
 std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
 while(flow < need) {
  std::fill(dist.begin(), dist.end(), INF), std::fill(vis.begin(), vis.end(), 0);
  dist[S]= 0, pq.push({0, S});
  while(!pq.empty()) {
   const auto [d, v]= pq.top();
   pq.pop();
   if(vis[v]) continue;
   vis[v]= 1;
   if(v == T) break;
   for(int k= 0; k < (int)g[v].size(); ++k) {
    const E& e= g[v][k];
    if(!e.cap || vis[e.to]) continue;
    const i64 nd= d + e.cost - dual[e.to] + dual[v];
    if(nd < dist[e.to]) dist[e.to]= nd, pv[e.to]= v, pe[e.to]= k, pq.push({nd, e.to});
   }
  }
  while(!pq.empty()) pq.pop();
  if(!vis[T]) break;
  for(int v= 0; v < N; ++v)
   if(vis[v]) dual[v]-= dist[T] - dist[v];
  i64 c= need - flow;
  for(int v= T; v != S; v= pv[v]) c= std::min(c, g[pv[v]][pe[v]].cap);
  for(int v= T; v != S; v= pv[v]) {
   E& e= g[pv[v]][pe[v]];
   e.cap-= c, g[v][e.rev].cap+= c;
  }
  flow+= c;
 }
 if(flow < need) return res;
 res.ok= true;
 res.pot.assign(dual.begin(), dual.begin() + n);
 res.flow.resize(m);
 for(int i= 0; i < m; ++i) {
  const auto& [s, t, l, u, c]= edges[i];
  if(pos[i].first < 0) res.flow[i]= c < 0 ? u : l;
  else {
   const E& e= g[pos[i].first][pos[i].second];
   res.flow[i]= l + g[e.to][e.rev].cap;
  }
  res.cost+= (__int128)res.flow[i] * c;
 }
 return res;
}
}
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_ssp::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_ssp::b_flow(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
