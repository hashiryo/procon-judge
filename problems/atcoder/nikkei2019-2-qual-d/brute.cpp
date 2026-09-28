// nikkei2019-2-qual-d の愚直解。操作ごとに L <= s < t <= R の組を全部たどって、頂点どうしの辺の長さの最小を
// 表 w[s][t] に書き、その完全グラフで O(N^2) のダイクストラ法を回す。区間をまとめる頂点 (セグメント木) を
// 作らないので、提出とは別の考え方になる。O(M N^2 + N^2) なので、N と M が 300 までのときだけ使う。
#include <cstdio>
#include <vector>

using i64 = long long;

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  const i64 INF = (i64)4e18;
  std::vector<std::vector<i64>> w(n + 1, std::vector<i64>(n + 1, INF));
  for (int i = 0; i < m; ++i) {
    int l, r;
    i64 c;
    if (scanf("%d %d %lld", &l, &r, &c) != 3) return 1;
    for (int s = l; s <= r; ++s)
      for (int t = s + 1; t <= r; ++t)
        if (c < w[s][t]) w[s][t] = w[t][s] = c;
  }
  std::vector<i64> dist(n + 1, INF);
  std::vector<char> done(n + 1, 0);
  dist[1] = 0;
  for (int it = 0; it < n; ++it) {
    int v = -1;
    for (int u = 1; u <= n; ++u)
      if (!done[u] && dist[u] < INF && (v < 0 || dist[u] < dist[v])) v = u;
    if (v < 0) break;
    done[v] = 1;
    for (int u = 1; u <= n; ++u)
      if (w[v][u] < INF && dist[v] + w[v][u] < dist[u]) dist[u] = dist[v] + w[v][u];
  }
  printf("%lld\n", dist[n] == INF ? -1LL : dist[n]);
}
