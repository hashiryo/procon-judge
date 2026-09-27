// abc231-h の愚直解。行と列の少ない方を列にして、行を 1 つずつ見る DP。どの行でも黒くする駒の集合
// (空でないもの) を全部試し、それまでに黒い駒がある列の集合を状態にして、最後に全部の列が入る最小の重みを出す。
// マトロイド交差を使わないので、提出とは別の考え方になる。O(行の数 4^列の数) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

int main() {
  int h, w, n;
  if (scanf("%d %d %d", &h, &w, &n) != 3) return 1;
  std::vector<int> a(n), b(n);
  std::vector<long long> c(n);
  for (int i = 0; i < n; ++i) {
    if (scanf("%d %d %lld", &a[i], &b[i], &c[i]) != 3) return 1;
    --a[i], --b[i];
  }
  if (w > h) std::swap(h, w), std::swap(a, b);  // 列の方を少なくする
  std::vector<std::vector<int>> row(h);
  for (int i = 0; i < n; ++i) row[a[i]].push_back(i);
  const long long INF = 1LL << 62;
  std::vector<long long> dp(1 << w, INF);
  dp[0] = 0;
  for (int r = 0; r < h; ++r) {
    int k = row[r].size();
    // その行で黒くする駒の集合ごとの、列の集合と重みの和。
    std::vector<int> cover(1 << k, 0);
    std::vector<long long> cost(1 << k, 0);
    for (int s = 1; s < (1 << k); ++s) {
      int j = __builtin_ctz(s), e = row[r][j];
      cover[s] = cover[s & (s - 1)] | 1 << b[e];
      cost[s] = cost[s & (s - 1)] + c[e];
    }
    std::vector<long long> next(1 << w, INF);
    for (int mask = 0; mask < (1 << w); ++mask) {
      if (dp[mask] == INF) continue;
      for (int s = 1; s < (1 << k); ++s) {
        long long &to = next[mask | cover[s]];
        to = std::min(to, dp[mask] + cost[s]);
      }
    }
    dp = std::move(next);
  }
  printf("%lld\n", dp[(1 << w) - 1]);
}
