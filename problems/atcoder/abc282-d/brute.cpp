// abc282-d の愚直解。辺の無い組 (u, v) をすべて試し、その辺を 1 本足したグラフ全体を毎回 BFS で
// 2 色に塗れるか調べて数える。塗り分けの色と連結成分の大きさから数を出す、という提出の考え方を使わない。
// O(N^2 (N + M)) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int n;
std::vector<std::vector<int>> adj;

bool bipartite() {
  std::vector<int> color(n, -1);
  for (int s = 0; s < n; ++s) {
    if (color[s] != -1) continue;
    color[s] = 0;
    std::vector<int> queue = {s};
    for (size_t head = 0; head < queue.size(); ++head) {
      int v = queue[head];
      for (int u : adj[v]) {
        if (color[u] == -1) color[u] = color[v] ^ 1, queue.push_back(u);
        else if (color[u] == color[v]) return false;
      }
    }
  }
  return true;
}

int main() {
  int m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  adj.assign(n, {});
  std::vector<std::vector<bool>> has(n, std::vector<bool>(n, false));
  for (int i = 0; i < m; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    adj[u].push_back(v), adj[v].push_back(u);
    has[u][v] = has[v][u] = true;
  }
  long long ans = 0;
  for (int u = 0; u < n; ++u)
    for (int v = u + 1; v < n; ++v) {
      if (has[u][v]) continue;
      adj[u].push_back(v), adj[v].push_back(u);
      ans += bipartite();
      adj[u].pop_back(), adj[v].pop_back();
    }
  printf("%lld\n", ans);
}
