#pragma once
// 全域最小カットの書き比べの核。Stoer-Wagner の方法を、隣接の列と 2 分ヒープで回す O(nm log n) の形。縮めた頂点は
// 併査集で代表に寄せ、隣接の列は代表の列に継ぎ足す (同じ相手への辺はまとめない)。ヒープは古い鍵を残して、取り出すときに
// 捨てる。段の数は頂点の数 - 1 で、段ごとに今の辺を全部なめる。
#include <array>
#include <queue>
#include <utility>
#include <vector>
namespace gmc {
inline long long sw_heap(int n, const std::vector<std::array<long long, 3>>& edges) {
 std::vector<std::vector<std::pair<int, long long>>> adj(n);
 for(auto& [u, v, c]: edges)
  if(u != v) adj[u].push_back({(int)v, c}), adj[v].push_back({(int)u, c});
 std::vector<int> par(n), alive;
 for(int i= 0; i < n; ++i) par[i]= i, alive.push_back(i);
 auto find= [&](int x) {
  while(par[x] != x) x= par[x]= par[par[x]];
  return x;
 };
 long long best= -1;
 std::vector<long long> a(n);
 std::vector<int> stamp(n, 0);
 int phase= 0;
 while(alive.size() > 1) {
  ++phase;
  for(int v: alive) a[v]= 0;
  std::priority_queue<std::pair<long long, int>> pq;
  for(int v: alive) pq.push({0, v});
  int prev= -1, last= -1;
  const int m= alive.size();
  for(int k= 0; k < m; ++k) {
   int sel;
   for(;;) {
    auto [key, v]= pq.top();
    pq.pop();
    if(stamp[v] != phase && key == a[v]) {
     sel= v;
     break;
    }
   }
   stamp[sel]= phase;
   if(k == m - 1) {
    last= sel;
    if(best < 0 || a[sel] < best) best= a[sel];
    break;
   }
   prev= sel;
   for(auto& [x, c]: adj[sel]) {
    const int y= find(x);
    if(y == sel || stamp[y] == phase) continue;
    a[y]+= c, pq.push({a[y], y});
   }
  }
  // last を prev へ縮める。
  par[last]= prev;
  auto& A= adj[prev];
  for(auto& e: adj[last]) A.push_back(e);
  std::vector<std::pair<int, long long>>().swap(adj[last]);
  for(int i= 0; i < (int)alive.size(); ++i)
   if(alive[i] == last) {
    alive[i]= alive.back(), alive.pop_back();
    break;
   }
 }
 return best < 0 ? 0 : best;
}
}  // namespace gmc
