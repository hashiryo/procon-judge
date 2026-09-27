// abc213-g の愚直解。残す辺の部分集合 H を 2^M 通り全部試し、Union-Find で頂点 1 とつながる頂点を数える。
// 連結なグラフの数を集合冪級数の log で数えないので、提出とは別の考え方になる。O(2^M (M + N)) なので、
// 辺の少ない入力でだけ使う。
#include <cstdio>
#include <vector>

int parent[17];

int find(int v) {
  while (parent[v] != v) v = parent[v] = parent[parent[v]];
  return v;
}

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2 || m > 24) return 1;
  std::vector<int> a(m), b(m);
  for (int i = 0; i < m; ++i) {
    if (scanf("%d %d", &a[i], &b[i]) != 2) return 1;
    --a[i], --b[i];
  }
  std::vector<long long> count(n, 0);
  for (long long mask = 0; mask < (1LL << m); ++mask) {
    for (int v = 0; v < n; ++v) parent[v] = v;
    for (int i = 0; i < m; ++i)
      if (mask >> i & 1) parent[find(a[i])] = find(b[i]);
    for (int k = 1; k < n; ++k)
      if (find(k) == find(0)) ++count[k];
  }
  for (int k = 1; k < n; ++k) printf("%lld\n", count[k] % 998244353);
}
