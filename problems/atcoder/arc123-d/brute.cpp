// arc123-d の愚直解。B_i の値ごとに、そこまでの |B_j| + |C_j| の和の最小を持つ DP。C が増えない
// ことは B_{i+1} - B_i >= max(0, A_{i+1} - A_i) と同じなので、1 つ前の B のうちこの幅以上小さい
// ものの最小に |B| + |A_i - B| を足す。区分線形凸関数を持ち回らないので、PiecewiseLinearConvex
// とは別の考え方になる。最適な B_i は [-(M + D), M + D] に入る (M = max |A_i|、D は上りの差の和。
// はみ出す B_i があれば、そこから先か手前をまとめて 1 ずらすと和が減る) ので、その範囲を全部持つ。
// N と |A_i| が小さいときだけ使う。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto& x : a)
    if (scanf("%lld", &x) != 1) return 1;
  long long m = 0, d = 0;
  for (int i = 0; i < n; ++i) m = std::max(m, std::llabs(a[i]));
  for (int i = 0; i + 1 < n; ++i) d += std::max(0LL, a[i + 1] - a[i]);
  const long long lo = -(m + d), width = 2 * (m + d) + 1;
  const long long INF = 1LL << 62;
  auto cost = [&](int i, long long b) { return std::llabs(b) + std::llabs(a[i] - b); };
  std::vector<long long> dp(width), next(width);
  for (long long k = 0; k < width; ++k) dp[k] = cost(0, lo + k);
  for (int i = 1; i < n; ++i) {
    long long rise = std::max(0LL, a[i] - a[i - 1]), best = INF;
    for (long long k = 0; k < width; ++k) {  // B_i = lo + k。B_{i-1} <= B_i - rise のものの最小
      if (k - rise >= 0) best = std::min(best, dp[k - rise]);
      next[k] = best == INF ? INF : best + cost(i, lo + k);
    }
    std::swap(dp, next);
  }
  printf("%lld\n", *std::min_element(dp.begin(), dp.end()));
}
