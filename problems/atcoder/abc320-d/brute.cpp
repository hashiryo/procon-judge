// abc320-d の愚直解。情報を両向きの辺にして人 1 から幅優先で進み、辺をたどるたびに差を足して座標を決める。
// 着かなかった人は undecidable。成分をまとめる木を持たないので、UnionFind_Potentialized とは別の考え方になる。
#include <cstdio>
#include <queue>
#include <vector>

struct Edge {
  int to;
  long long dx, dy;
};

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<std::vector<Edge>> g(n);
  for (int i = 0; i < m; ++i) {
    int a, b;
    long long x, y;
    if (scanf("%d %d %lld %lld", &a, &b, &x, &y) != 4) return 1;
    --a, --b;
    g[a].push_back({b, x, y});  // B は A から見て (x, y)
    g[b].push_back({a, -x, -y});
  }
  std::vector<bool> seen(n, false);
  std::vector<long long> sx(n, 0), sy(n, 0);
  std::queue<int> q;
  seen[0] = true;
  q.push(0);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (const Edge& e : g[u])
      if (!seen[e.to]) seen[e.to] = true, sx[e.to] = sx[u] + e.dx, sy[e.to] = sy[u] + e.dy, q.push(e.to);
  }
  for (int i = 0; i < n; ++i) {
    if (seen[i]) printf("%lld %lld\n", sx[i], sy[i]);
    else puts("undecidable");
  }
}
