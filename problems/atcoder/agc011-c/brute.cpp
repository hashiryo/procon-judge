// agc011-c の愚直解。頂点 (a, b) を N^2 個並べ、元の辺の組ごとに新しいグラフの辺を張って、幅優先で連結成分を数える。
// 2 部グラフかどうかで数え分ける式を使わないので、UnionFind_Potentialized とは別の考え方になる。
// O(N^2 + M^2) なので小さい入力でだけ使う。
#include <cstdio>
#include <queue>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<std::pair<int, int>> edges(m);
  for (auto& [u, v] : edges) {
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
  }
  auto id = [&](int a, int b) { return a * n + b; };
  std::vector<std::vector<int>> g(n * n);
  // (a, b) と (a', b') は、a-a' と b-b' がどちらも元の辺のときにつながる。
  for (auto [a, a2] : edges)
    for (auto [b, b2] : edges) {
      g[id(a, b)].push_back(id(a2, b2)), g[id(a2, b2)].push_back(id(a, b));
      g[id(a, b2)].push_back(id(a2, b)), g[id(a2, b)].push_back(id(a, b2));
    }
  std::vector<bool> seen(n * n, false);
  long long components = 0;
  for (int s = 0; s < n * n; ++s) {
    if (seen[s]) continue;
    ++components;
    seen[s] = true;
    std::queue<int> q;
    q.push(s);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (int v : g[u])
        if (!seen[v]) seen[v] = true, q.push(v);
    }
  }
  printf("%lld\n", components);
}
