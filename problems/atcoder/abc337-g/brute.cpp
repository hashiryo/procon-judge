// abc337-g の愚直解。u ごとに u を根にして親を求め、v ごとに v から u まで親をたどって、道の上の w > v を数える。
// 全方位木 DP も WaveletMatrix での数え上げもしないので、提出とは別の考え方になる。O(N^3) なので、
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
  for (int u = 1; u <= n; ++u) {
    // u を根にした親。幅優先でたどる。
    std::vector<int> parent(n + 1, -1), bfs = {u};
    parent[u] = 0;
    for (size_t i = 0; i < bfs.size(); ++i)
      for (int x : adj[bfs[i]])
        if (parent[x] < 0) parent[x] = bfs[i], bfs.push_back(x);
    long long f = 0;
    for (int v = 1; v <= n; ++v)
      for (int w = v; w != 0; w = parent[w])  // v から u までの道の頂点 w (両端を含む)
        if (w > v) ++f;
    printf("%lld%c", f, u == n ? '\n' : ' ');
  }
}
