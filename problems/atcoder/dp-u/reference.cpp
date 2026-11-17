// dp-u の期待出力を作る参照実装 (ライブラリに依存しない)。塊の得点 w[T] を前から作り、dp[S] = max_{T ⊆ S、T は S の最小の
// 要素を含む} w[T] + dp[S \ T] を 3^N で回す。小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<long long>> a(n, std::vector<long long>(n));
  for (auto &row : a)
    for (auto &v : row)
      if (scanf("%lld", &v) != 1) return 1;
  const int N = 1 << n;
  std::vector<long long> w(N, 0), dp(N, 0);
  for (int s = 1; s < N; ++s) {
    const int hi = 31 - __builtin_clz(s), rest = s ^ (1 << hi);
    w[s] = w[rest];
    for (int j = 0; j < hi; ++j)
      if (rest >> j & 1) w[s] += a[hi][j];
  }
  for (int s = 1; s < N; ++s) {
    const int lo = s & -s, rest = s ^ lo;
    long long best = -(1LL << 62);
    for (int t = rest;; t = (t - 1) & rest) {
      best = std::max(best, w[lo | t] + dp[rest ^ t]);
      if (!t) break;
    }
    dp[s] = best;
  }
  printf("%lld\n", dp[N - 1]);
}
