// abc220-f の愚直解。頂点 i ごとに i から幅優先探索して、全部の頂点までの距離を足す。
// 部分木の大きさを使った根の付け替えをしないので、Rerooting とは別の考え方になる。O(N^2) なので、
// 小さい木でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<int>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    adj[u].push_back(v), adj[v].push_back(u);
  }
  for (int s = 1; s <= n; ++s) {
    std::vector<int> dist(n + 1, -1), bfs = {s};
    dist[s] = 0;
    long long total = 0;
    for (size_t i = 0; i < bfs.size(); ++i) {
      int v = bfs[i];
      total += dist[v];
      for (int u : adj[v])
        if (dist[u] < 0) dist[u] = dist[v] + 1, bfs.push_back(u);
    }
    printf("%lld\n", total);
  }
}
