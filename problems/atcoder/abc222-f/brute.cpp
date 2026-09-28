// abc222-f の愚直解。頂点 i ごとに i から木全体をたどって距離を求め、j ≠ i の距離 + D_j の最大を出す。
// 全方位木 DP をしないので、Rerooting とは別の考え方になる。O(N^2) なので、小さい木でだけ使う。
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<std::pair<int, long long>>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int a, b;
    long long c;
    if (scanf("%d %d %lld", &a, &b, &c) != 3) return 1;
    adj[a].emplace_back(b, c), adj[b].emplace_back(a, c);
  }
  std::vector<long long> d(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%lld", &d[i]) != 1) return 1;
  for (int i = 1; i <= n; ++i) {
    std::vector<long long> dist(n + 1, -1);
    std::vector<int> stack = {i};
    dist[i] = 0;
    while (!stack.empty()) {
      int v = stack.back();
      stack.pop_back();
      for (auto [u, c] : adj[v])
        if (dist[u] < 0) dist[u] = dist[v] + c, stack.push_back(u);
    }
    long long best = 0;
    for (int j = 1; j <= n; ++j)
      if (j != i) best = std::max(best, dist[j] + d[j]);
    printf("%lld\n", best);
  }
}
