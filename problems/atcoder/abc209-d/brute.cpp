// abc209-d の愚直解。質問ごとに c から幅優先探索して d までの距離を求め、偶数なら Town、奇数なら Road。
// 重み付き UnionFind で偶奇を持たないので、提出とは別の考え方になる。O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <queue>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<std::vector<int>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    adj[a].push_back(b), adj[b].push_back(a);
  }
  while (q--) {
    int c, d;
    if (scanf("%d %d", &c, &d) != 2) return 1;
    std::vector<int> dist(n + 1, -1);
    std::queue<int> bfs;
    dist[c] = 0, bfs.push(c);
    while (!bfs.empty()) {
      int v = bfs.front();
      bfs.pop();
      for (int u : adj[v])
        if (dist[u] < 0) dist[u] = dist[v] + 1, bfs.push(u);
    }
    puts(dist[d] % 2 == 0 ? "Town" : "Road");
  }
}
