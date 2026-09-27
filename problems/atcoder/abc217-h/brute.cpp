// abc217-h の愚直解。位置 p (|p| <= T_N) ごとに「今その位置に居るときの、それまでの最小ダメージ」を持ち、
// 1 秒ずつ隣の 3 マスの最小を取って進め、撃たれる時刻にダメージを足す DP。
// 凸関数の傾きと折れ点を扱わないので、PiecewiseLinearConvex とは別の考え方になる。
// O(T_N^2) なので T_N が小さいときだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> t(n), x(n);
  std::vector<int> d(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %d %lld", &t[i], &d[i], &x[i]) != 3) return 1;
  const long long r = t[n - 1];
  const long long INF = 1LL << 62;
  // dp[p + r] = 位置 p に居るときの最小ダメージ。まだ届かない位置は INF。
  std::vector<long long> dp(2 * r + 1, INF), next(2 * r + 1);
  dp[r] = 0;
  long long now = 0;
  for (int i = 0; i < n; ++i) {
    for (; now < t[i]; ++now) {
      for (long long k = 0; k <= 2 * r; ++k) {
        long long best = dp[k];
        if (k > 0) best = std::min(best, dp[k - 1]);
        if (k < 2 * r) best = std::min(best, dp[k + 1]);
        next[k] = best;
      }
      std::swap(dp, next);
    }
    for (long long k = 0; k <= 2 * r; ++k) {
      if (dp[k] == INF) continue;
      long long p = k - r;
      if (d[i] == 0 && p < x[i]) dp[k] += x[i] - p;
      if (d[i] == 1 && x[i] < p) dp[k] += p - x[i];
    }
  }
  printf("%lld\n", *std::min_element(dp.begin(), dp.end()));
}
