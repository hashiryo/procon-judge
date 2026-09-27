// abc201-e の愚直解。頂点 i ごとに i から木をたどって、全部の頂点 j への XOR 距離を求め、
// i < j の組の距離をそのまま (10^9 + 7 で割った余りで) 足す。根からの XOR をビットごとに数える
// 形にしないので、重み付き UnionFind を使う提出とは別の考え方になる。O(N^2) なので、小さい入力でだけ使う。
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  const unsigned long long MOD = 1000000007;
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<std::pair<int, unsigned long long>>> adj(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int u, v;
    unsigned long long w;
    if (scanf("%d %d %llu", &u, &v, &w) != 3) return 1;
    adj[u].push_back({v, w}), adj[v].push_back({u, w});
  }
  unsigned long long sum = 0;
  for (int i = 1; i <= n; ++i) {
    std::vector<unsigned long long> dist(n + 1);
    std::vector<bool> seen(n + 1);
    std::vector<int> stack = {i};
    seen[i] = true;
    while (!stack.empty()) {
      int v = stack.back();
      stack.pop_back();
      for (auto [u, w] : adj[v])
        if (!seen[u]) seen[u] = true, dist[u] = dist[v] ^ w, stack.push_back(u);
    }
    for (int j = i + 1; j <= n; ++j) sum = (sum + dist[j] % MOD) % MOD;
  }
  printf("%llu\n", sum);
}
