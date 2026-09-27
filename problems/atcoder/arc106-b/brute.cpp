// arc106-b の愚直解。隣接行列から推移閉包 (Warshall) を作って、頂点ごとに「行ける頂点の a - b の和」が
// 0 かを調べる。操作は辺の両端の和を変えないので、どの頂点から見ても行ける範囲の和が 0 なら Yes。
// 辺ごとの流量を全域森の上で解かないので、提出とは別の考え方になる。O(N^3) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2 || n > 1000) return 1;
  std::vector<long long> a(n), b(n);
  for (auto &x : a)
    if (scanf("%lld", &x) != 1) return 1;
  for (auto &x : b)
    if (scanf("%lld", &x) != 1) return 1;
  std::vector<std::vector<char>> reach(n, std::vector<char>(n, 0));
  for (int i = 0; i < n; ++i) reach[i][i] = 1;
  for (int i = 0; i < m; ++i) {
    int c, d;
    if (scanf("%d %d", &c, &d) != 2) return 1;
    reach[c - 1][d - 1] = reach[d - 1][c - 1] = 1;
  }
  for (int k = 0; k < n; ++k)
    for (int i = 0; i < n; ++i)
      if (reach[i][k])
        for (int j = 0; j < n; ++j)
          if (reach[k][j]) reach[i][j] = 1;
  bool ok = true;
  for (int i = 0; i < n; ++i) {
    long long sum = 0;
    for (int j = 0; j < n; ++j)
      if (reach[i][j]) sum += a[j] - b[j];
    if (sum != 0) ok = false;
  }
  puts(ok ? "Yes" : "No");
}
