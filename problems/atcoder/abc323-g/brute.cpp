// abc323-g の愚直解。長さ N - 2 の Prüfer 列を全部たどって全域木に戻し、辺 (u, v) (u < v) のうち
// P_u > P_v のものを数えて、本数ごとに木を数える。行列木定理も、x の 1 次式を成分に持つ行列の行列式も
// 使わないので、提出とは別の考え方になる。N^(N-2) 本の木をたどるので、小さい N でだけ使う。
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> p(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%d", &p[i]) != 1) return 1;
  std::vector<long long> count(n, 0);
  int len = n - 2;
  std::vector<int> code(len, 1);  // Prüfer 列。各項は 1 から n
  while (true) {
    // Prüfer 列から木に戻す。毎回、残っている葉のうち番号が最小のものを列の頭と繋ぐ。
    std::vector<int> degree(n + 1, 1);
    for (int x : code) ++degree[x];
    int inversions = 0;
    auto add_edge = [&](int u, int v) {
      if (u > v) std::swap(u, v);
      if (p[u] > p[v]) ++inversions;
    };
    for (int x : code) {
      int leaf = 1;
      while (degree[leaf] != 1) ++leaf;
      add_edge(leaf, x);
      --degree[leaf], --degree[x];
    }
    int u = 1;
    while (degree[u] != 1) ++u;
    int v = u + 1;
    while (degree[v] != 1) ++v;
    add_edge(u, v);
    ++count[inversions];
    // 次の Prüfer 列 (n 進数として 1 を足す)。
    int i = 0;
    while (i < len && code[i] == n) code[i++] = 1;
    if (i == len) break;
    ++code[i];
  }
  for (int k = 0; k < n; ++k) printf("%lld%c", count[k] % 998244353, k + 1 == n ? '\n' : ' ');
}
