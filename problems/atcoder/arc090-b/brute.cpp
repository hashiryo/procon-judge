// arc090-b (People on a Line) の愚直解。x_R - x_L = D を
// x_R - x_L <= D と x_L - x_R <= -D の 2 つの不等式にし、0 <= x_i <= 10^9 も x_0 = 0 と置いた頂点 0 との不等式にして、
// Floyd–Warshall で負の閉路があるかを調べる。成分ごとに差をまとめる木を持たないので、UnionFind_Potentialized とは
// 別の考え方になる。O(N^3) なので小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  const long long INF = 1LL << 60, BOUND = 1000000000;
  // dist[i][j] は x_j - x_i の上限。
  std::vector<std::vector<long long>> dist(n + 1, std::vector<long long>(n + 1, INF));
  for (int i = 0; i <= n; ++i) dist[i][i] = 0;
  for (int i = 1; i <= n; ++i) dist[0][i] = BOUND, dist[i][0] = 0;
  for (int k = 0; k < m; ++k) {
    int l, r;
    long long d;
    if (scanf("%d %d %lld", &l, &r, &d) != 3) return 1;
    dist[l][r] = std::min(dist[l][r], d);
    dist[r][l] = std::min(dist[r][l], -d);
  }
  // 負の閉路があると値がどんどん小さくなるので、-INF で止めてあふれないようにする。
  for (int k = 0; k <= n; ++k)
    for (int i = 0; i <= n; ++i)
      for (int j = 0; j <= n; ++j)
        if (dist[i][k] < INF && dist[k][j] < INF) dist[i][j] = std::max(-INF, std::min(dist[i][j], dist[i][k] + dist[k][j]));
  bool ok = true;
  for (int i = 0; i <= n; ++i) ok &= dist[i][i] >= 0;
  puts(ok ? "Yes" : "No");
}
