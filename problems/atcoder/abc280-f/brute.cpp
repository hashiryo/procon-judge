// abc280-f の愚直解。道 i を A_i -> B_i (+C_i) と B_i -> A_i (-C_i) の 2 本の有向辺にして、質問ごとに X から
// Bellman-Ford で最長路を求める。N - 1 周で落ち着かずにまだ伸びる町は正の閉路から行けるので inf にし、
// inf は辺に沿って広げる。町に高さ (ポテンシャル) を置かないので、UnionFind_Potentialized とは別の考え方になる。
// 質問 1 つが O(NM) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m, q;
  if (scanf("%d %d %d", &n, &m, &q) != 3) return 1;
  struct Edge { int from, to; long long w; };
  std::vector<Edge> edges;
  for (int i = 0; i < m; ++i) {
    int a, b;
    long long c;
    if (scanf("%d %d %lld", &a, &b, &c) != 3) return 1;
    edges.push_back({a, b, c}), edges.push_back({b, a, -c});
  }
  const long long NONE = -(1LL << 62);
  while (q--) {
    int x, y;
    if (scanf("%d %d", &x, &y) != 2) return 1;
    std::vector<long long> best(n + 1, NONE);
    best[x] = 0;
    for (int round = 0; round < n - 1; ++round)
      for (auto& e : edges)
        if (best[e.from] != NONE && best[e.from] + e.w > best[e.to]) best[e.to] = best[e.from] + e.w;
    // まだ伸びる町と、そこから行ける町は inf。n 周すれば全部に広がる。
    std::vector<bool> inf(n + 1, false);
    for (int round = 0; round < n; ++round)
      for (auto& e : edges) {
        if (best[e.from] == NONE) continue;
        if (best[e.from] + e.w > best[e.to]) best[e.to] = best[e.from] + e.w, inf[e.to] = true;
        if (inf[e.from]) inf[e.to] = true;
      }
    if (best[y] == NONE) puts("nan");
    else if (inf[y]) puts("inf");
    else printf("%lld\n", best[y]);
  }
}
