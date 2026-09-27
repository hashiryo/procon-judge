// arc105-f の愚直解。残す辺の集合を 2^M 通り全部試し、全部の頂点が繋がっていて、2 色に塗り分けられる
// (二部グラフ) ものを数える。操作で全部の辺を青にできることは、各頂点を操作した回数の偶奇で 2 色に
// 塗ると辺の両端の色が違うことと同じ。頂点の部分集合の冪級数 (畳み込みと log) を使わないので、
// 提出とは別の考え方になる。O(2^M (N + M)) なので、辺の少ないグラフでだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<int> a(m), b(m);
  for (int i = 0; i < m; ++i) {
    if (scanf("%d %d", &a[i], &b[i]) != 2) return 1;
    --a[i], --b[i];
  }
  long long count = 0;
  for (long long mask = 0; mask < (1LL << m); ++mask) {
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < m; ++i)
      if (mask >> i & 1) adj[a[i]].push_back(b[i]), adj[b[i]].push_back(a[i]);
    // 頂点 0 から幅優先で 2 色に塗る。
    std::vector<int> color(n, -1), bfs = {0};
    color[0] = 0;
    bool ok = true;
    for (size_t i = 0; i < bfs.size() && ok; ++i)
      for (int v : adj[bfs[i]]) {
        if (color[v] < 0) color[v] = color[bfs[i]] ^ 1, bfs.push_back(v);
        else if (color[v] == color[bfs[i]]) ok = false;
      }
    if (ok && (int)bfs.size() == n) ++count;
  }
  printf("%lld\n", count % 998244353);
}
