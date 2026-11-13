// 期待出力を作る参照実装。大学と分野を頂点にし、人を大学から分野への容量 1、費用 -C の辺にした網で、s から t へ逐次最短路で
// 1 本ずつ流す (教科書どおりの形。初めのポテンシャルは Bellman-Ford、そのあとは std::priority_queue の Dijkstra)。i 本目の
// 道を流したあとの費用の符号を反転したものが、i 人のときの答え。ライブラリを include しない (期待出力のキャッシュの鍵は
// このファイルの中身だけで決まる)。
#include <algorithm>
#include <cstdio>
#include <limits>
#include <queue>
#include <vector>
using namespace std;
using i64 = long long;
struct Arc {
  int to, rev;
  int cap;
  i64 cost;
};
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  const int A = 150, S = 0, T = 2 * A + 1, V = 2 * A + 2;
  vector<vector<Arc>> g(V);
  auto add = [&](int u, int v, int cap, i64 cost) {
    g[u].push_back({v, (int)g[v].size(), cap, cost});
    g[v].push_back({u, (int)g[u].size() - 1, 0, -cost});
  };
  for (int a = 1; a <= A; ++a) add(S, a, 1, 0), add(A + a, T, 1, 0);
  for (int i = 0; i < n; ++i) {
    int a, b;
    i64 c;
    if (scanf("%d %d %lld", &a, &b, &c) != 3) return 1;
    add(a, A + b, 1, -c);
  }
  const i64 INF = numeric_limits<i64>::max() / 4;
  vector<i64> p(V, 0);
  for (int it = 0; it < V; ++it) {
    bool ch = false;
    for (int u = 0; u < V; ++u)
      for (const Arc &e : g[u])
        if (e.cap > 0 && p[u] + e.cost < p[e.to]) p[e.to] = p[u] + e.cost, ch = true;
    if (!ch) break;
  }
  vector<i64> ans;
  i64 total = 0;
  vector<i64> dist(V);
  vector<pair<int, int>> prv(V);
  while (true) {
    fill(dist.begin(), dist.end(), INF);
    priority_queue<pair<i64, int>, vector<pair<i64, int>>, greater<>> pq;
    dist[S] = 0, pq.push({0, S});
    while (!pq.empty()) {
      auto [d, u] = pq.top();
      pq.pop();
      if (d > dist[u]) continue;
      for (int k = 0; k < (int)g[u].size(); ++k) {
        const Arc &e = g[u][k];
        if (e.cap <= 0) continue;
        const i64 nd = d + e.cost + p[u] - p[e.to];
        if (nd < dist[e.to]) dist[e.to] = nd, prv[e.to] = {u, k}, pq.push({nd, e.to});
      }
    }
    if (dist[T] == INF) break;
    for (int v = 0; v < V; ++v)
      if (dist[v] < INF) p[v] += dist[v];
    i64 c = 0;
    for (int v = T; v != S; v = prv[v].first) {
      Arc &e = g[prv[v].first][prv[v].second];
      e.cap -= 1, g[v][e.rev].cap += 1, c += e.cost;
    }
    total += c;
    ans.push_back(-total);
  }
  printf("%zu\n", ans.size());
  for (i64 x : ans) printf("%lld\n", x);
}
