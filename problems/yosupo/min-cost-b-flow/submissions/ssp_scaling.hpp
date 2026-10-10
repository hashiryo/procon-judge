#pragma once
// 容量のスケーリングを入れた逐次最短路 (Ahuja, Magnanti, Orlin の capacity scaling)。下限を先に流して、各頂点の過不足
// ex を求める。Δ を容量と過不足の最大以下の 2 の冪から始めて半分ずつにする。各 Δ の初めに、残余容量が Δ 以上で縮約
// 費用 c + p_u - p_v が負の辺を上限まで流し、そのうえで、過剰が Δ 以上の頂点すべてを始点にした Dijkstra (残余容量が
// Δ 以上の辺だけ) で、不足が Δ 以上の頂点への最短路を探して Δ だけ流すことを、どちらかが無くなるまで繰り返す。
// ポテンシャルは ssp と同じく、Dijkstra で訪れた頂点だけを動かす。Δ = 1 の段を終えて過不足が残れば、b-flow は無い。
// 各 Δ の増加の回数は O(n + m) に収まるので、逐次最短路に指数回の増加をさせる入力でも止まる。自己ループは、費用が
// 負なら上限、それ以外は下限まで流したまま残余グラフに入れない。
#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <vector>
namespace mcf_ssp_scaling {
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
 const int m= edges.size();
 std::vector<i64> ex(b.begin(), b.end());
 std::vector<std::vector<E>> g(n);
 std::vector<std::pair<int, int>> pos(m, {-1, -1});
 i64 mx= 1;
 for(int i= 0; i < m; ++i) {
  const auto& [s, t, l, u, c]= edges[i];
  if(s == t) continue;
  ex[s]-= l, ex[t]+= l;
  pos[i]= {(int)s, (int)g[s].size()};
  g[s].push_back({(int)t, (int)g[t].size(), u - l, c});
  g[t].push_back({(int)s, (int)g[s].size() - 1, 0, -c});
  mx= std::max(mx, u - l);
 }
 Result res;
 {
  i64 sum= 0;
  for(int v= 0; v < n; ++v) sum+= ex[v], mx= std::max(mx, ex[v] < 0 ? -ex[v] : ex[v]);
  if(sum != 0) return res;
 }
 const i64 INF= std::numeric_limits<i64>::max() / 4;
 std::vector<i64> p(n, 0), dist(n);
 std::vector<int> pv(n), pe(n);
 std::vector<char> vis(n);
 using P= std::pair<i64, int>;
 std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
 i64 D= 1;
 while(D * 2 <= mx) D*= 2;
 for(; D >= 1; D/= 2) {
  // 縮約費用が負で、残余容量が D 以上の辺を飽和させる。
  for(int v= 0; v < n; ++v)
   for(E& e: g[v])
    if(e.cap >= D && e.cost + p[v] - p[e.to] < 0) {
     const i64 c= e.cap;
     e.cap= 0, g[e.to][e.rev].cap+= c;
     ex[v]-= c, ex[e.to]+= c;
    }
  for(;;) {
   std::fill(dist.begin(), dist.end(), INF), std::fill(vis.begin(), vis.end(), 0);
   bool any= false;
   for(int v= 0; v < n; ++v)
    if(ex[v] >= D) dist[v]= 0, pv[v]= -1, pq.push({0, v}), any= true;
   if(!any) break;
   int t= -1;
   while(!pq.empty()) {
    const auto [d, v]= pq.top();
    pq.pop();
    if(vis[v]) continue;
    vis[v]= 1;
    if(ex[v] <= -D) {
     t= v;
     break;
    }
    for(int k= 0; k < (int)g[v].size(); ++k) {
     const E& e= g[v][k];
     if(e.cap < D || vis[e.to]) continue;
     const i64 nd= d + e.cost + p[v] - p[e.to];
     if(nd < dist[e.to]) dist[e.to]= nd, pv[e.to]= v, pe[e.to]= k, pq.push({nd, e.to});
    }
   }
   while(!pq.empty()) pq.pop();
   if(t < 0) break;
   for(int v= 0; v < n; ++v)
    if(vis[v]) p[v]+= dist[v] - dist[t];
   int v= t;
   for(; pv[v] >= 0; v= pv[v]) {
    E& e= g[pv[v]][pe[v]];
    e.cap-= D, g[v][e.rev].cap+= D;
   }
   ex[v]-= D, ex[t]+= D;
  }
 }
 for(int v= 0; v < n; ++v)
  if(ex[v] != 0) return res;
 res.ok= true;
 res.pot= p;
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
 mcf_ssp_scaling::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_ssp_scaling::b_flow(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
