// abc133-f の愚直解。質問ごとに u から木をたどって各頂点の親の辺を求め、v から u まで戻りながら、
// 色が x の辺は y、それ以外は元の長さを足す。根からの長さの和も、色ごとの本数と長さの和を持つ
// 永続配列も LCA も使わないので、提出とは別の考え方になる。O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

struct Edge {
  int to, color, length;
};

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<std::vector<Edge>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int a, b, c, d;
    if (scanf("%d %d %d %d", &a, &b, &c, &d) != 4) return 1;
    adj[a].push_back({b, c, d}), adj[b].push_back({a, c, d});
  }
  while (q--) {
    int x, y, u, v;
    if (scanf("%d %d %d %d", &x, &y, &u, &v) != 4) return 1;
    // up[w]: w から u の側へ 1 つ戻る辺。
    std::vector<Edge> up(n + 1, {0, 0, 0});
    std::vector<bool> seen(n + 1);
    std::vector<int> stack = {u};
    seen[u] = true;
    while (!stack.empty()) {
      int w = stack.back();
      stack.pop_back();
      for (const Edge &e : adj[w])
        if (!seen[e.to]) seen[e.to] = true, up[e.to] = {w, e.color, e.length}, stack.push_back(e.to);
    }
    long long dist = 0;
    for (int w = v; w != u; w = up[w].to) dist += up[w].color == x ? y : up[w].length;
    printf("%lld\n", dist);
  }
}
