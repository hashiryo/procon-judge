// abc218-h の愚直解。ランプを左から順に見て、(ここまでに赤にした数, 直前のランプの色) を状態にする DP。
// 赤どうしが隣り合わない形への言い換えも、罰金で赤の数の制約を外す Alien DP もしないので、提出とは
// 別の考え方になる。O(NR) なので、小さい入力でだけ使う。
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>

int main() {
  int n, r;
  if (scanf("%d %d", &n, &r) != 2) return 1;
  std::vector<long long> a(n - 1);
  for (auto& x : a)
    if (scanf("%lld", &x) != 1) return 1;
  const long long NONE = -1;
  // dp[k][c]: ここまでのランプで赤を k 個にし、最後のランプの色が c (1 が赤) のときの報酬の最大。
  std::vector<std::array<long long, 2>> dp(r + 1, {NONE, NONE});
  dp[0][0] = 0, dp[1][1] = 0;
  for (int i = 1; i < n; ++i) {  // ランプ i を塗る。ランプ i - 1 との間の報酬は a[i - 1]
    std::vector<std::array<long long, 2>> next(r + 1, {NONE, NONE});
    for (int k = 0; k <= r; ++k)
      for (int c = 0; c < 2; ++c) {
        if (dp[k][c] == NONE) continue;
        for (int d = 0; d < 2; ++d) {
          if (k + d > r) continue;
          long long v = dp[k][c] + (c != d ? a[i - 1] : 0);
          next[k + d][d] = std::max(next[k + d][d], v);
        }
      }
    dp = std::move(next);
  }
  printf("%lld\n", std::max(dp[r][0], dp[r][1]));
}
