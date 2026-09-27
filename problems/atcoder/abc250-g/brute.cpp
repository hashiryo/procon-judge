// abc250-g の愚直解。dp[k] = その日までで株を k 株持っているときの、お金の増えた量の最大、を 1 日ずつ
// 「何もしない」「1 株買う」「1 株売る」の 3 通りから更新し、最後の日の dp の最大を答えにする。
// 凸関数の傾きや折れ点 (ヒープ) を扱わないので、PiecewiseLinearConvex とは別の考え方になる。
// O(N^2) なので小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  const long long NONE = -(1LL << 62);
  std::vector<long long> dp(n + 2, NONE), next(n + 2);
  dp[0] = 0;
  for (int i = 0; i < n; ++i) {
    long long p;
    if (scanf("%lld", &p) != 1) return 1;
    for (int k = 0; k <= n; ++k) {
      long long best = dp[k];
      if (k > 0 && dp[k - 1] != NONE) best = std::max(best, dp[k - 1] - p);
      if (dp[k + 1] != NONE) best = std::max(best, dp[k + 1] + p);
      next[k] = best;
    }
    next[n + 1] = NONE;
    std::swap(dp, next);
  }
  printf("%lld\n", *std::max_element(dp.begin(), dp.end()));
}
