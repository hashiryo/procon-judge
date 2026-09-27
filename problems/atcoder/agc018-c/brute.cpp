// agc018-c の愚直解。人を順に見て、金と銀をそれぞれ何人から取ったかを状態にする DP
// (銅の人数は残りで決まる)。双対の変数を探さないので、min_Lconvex とは別の考え方になる。
// 計算量は O((X+Y+Z) X Y) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int x, y, z;
  if (scanf("%d %d %d", &x, &y, &z) != 3) return 1;
  int n = x + y + z;
  const long long NONE = -1;
  // dp[a][b]: ここまでの人で、金を a 人、銀を b 人から取ったときの最大 (銅は残りの人)。
  std::vector<std::vector<long long>> dp(x + 1, std::vector<long long>(y + 1, NONE));
  dp[0][0] = 0;
  for (int i = 0; i < n; ++i) {
    long long coin[3];
    if (scanf("%lld %lld %lld", &coin[0], &coin[1], &coin[2]) != 3) return 1;
    auto next = std::vector<std::vector<long long>>(x + 1, std::vector<long long>(y + 1, NONE));
    for (int a = 0; a <= x; ++a)
      for (int b = 0; b <= y; ++b) {
        if (dp[a][b] == NONE) continue;
        int c = i - a - b;
        if (a < x) next[a + 1][b] = std::max(next[a + 1][b], dp[a][b] + coin[0]);
        if (b < y) next[a][b + 1] = std::max(next[a][b + 1], dp[a][b] + coin[1]);
        if (c < z) next[a][b] = std::max(next[a][b], dp[a][b] + coin[2]);
      }
    dp = std::move(next);
  }
  printf("%lld\n", dp[x][y]);
}
