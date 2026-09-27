// abc160-f の愚直解。N が 16 以下なら、根 k ごとに、番号を書き終えた頂点の集合を状態にして、
// その隣の頂点に次の番号を書く遷移を全部たどる DP で、書き方の数をそのまま数える。
// N が 17 以上なら、根 k ごとに木をたどって部分木の大きさを求め、N! / (部分木の大きさの積) を出す。
// どちらも根ごとに数え直し、全方位木 DP (Rerooting) も二項係数での合成も使わないので、提出とは別の考え方になる。
// 前者は O(N^2 2^N)、後者は O(N^2) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
constexpr u64 MOD = 1000000007;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= MOD; e; e >>= 1, x = x * x % MOD)
    if (e & 1) r = r * x % MOD;
  return r;
}

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<int>> adj(n);
  for (int i = 0; i < n - 1; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    --a, --b;
    adj[a].push_back(b), adj[b].push_back(a);
  }
  if (n <= 16) {
    std::vector<unsigned> near(n);
    for (int v = 0; v < n; ++v)
      for (int u : adj[v]) near[v] |= 1u << u;
    const unsigned full = (1u << n) - 1;
    for (int k = 0; k < n; ++k) {
      // ways[S]: 集合 S の頂点に 1 から |S| を書き終える書き方の数 (15! < 2^64 なので割らずに数える)。
      std::vector<u64> ways(full + 1);
      ways[1u << k] = 1;
      for (unsigned s = 1; s <= full; ++s) {
        if (!ways[s]) continue;
        unsigned next = 0;
        for (int v = 0; v < n; ++v)
          if (s >> v & 1) next |= near[v];
        next &= ~s;
        for (int v = 0; v < n; ++v)
          if (next >> v & 1) ways[s | 1u << v] += ways[s];
      }
      printf("%llu\n", ways[full] % MOD);
    }
    return 0;
  }
  u64 fact = 1;
  for (int i = 1; i <= n; ++i) fact = fact * i % MOD;
  for (int k = 0; k < n; ++k) {
    // k から幅優先でたどり、逆順に部分木の大きさを足し上げる。
    std::vector<int> order = {k}, parent(n, -1), size(n, 1);
    parent[k] = k;
    for (size_t i = 0; i < order.size(); ++i)
      for (int u : adj[order[i]])
        if (parent[u] < 0) parent[u] = order[i], order.push_back(u);
    u64 prod = 1;
    for (int i = n; i--;) {
      int v = order[i];
      if (v != k) size[parent[v]] += size[v];
      prod = prod * size[v] % MOD;
    }
    printf("%llu\n", fact * pow_mod(prod, MOD - 2) % MOD);
  }
}
