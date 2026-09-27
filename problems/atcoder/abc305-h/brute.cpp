// abc305-h の愚直解。区間 [i, j) を 1 日で解く最小の疲労を、区間の問題の部分集合 S ごとの最小
// best[S] = min_{t ∈ S} A_t × best[S \ {t}] + B_t (最後に解く問題 t で分ける) で求める。A_t >= 1 なので、
// 最後の問題の前の疲労は小さいほどよい。そのうえで、k 日で解くときの最小の疲労を日数ごとの DP で求め、
// X 以下になる最小の k を D とする。比で並べ替える交換の議論も、A = 1 の問題を先に取り除くことも、
// 日数に p を足すラグランジュの緩和もしないので、提出とは別の考え方になる。X を超えた値は X + 1 に
// 丸める (1 日でも X を超えれば全体も X を超える)。区間の長さについて指数時間なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  long long x;
  if (scanf("%d %lld", &n, &x) != 2) return 1;
  std::vector<long long> a(n), b(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %lld", &a[i], &b[i]) != 2) return 1;
  const long long over = x + 1;
  // cost[i][len]: [i, i + len) を 1 日で解く最小の疲労 (X を超えたら over)。
  std::vector<std::vector<long long>> cost(n);
  for (int i = 0; i < n; ++i) {
    std::vector<long long> best = {0};
    for (int len = 1; i + len <= n; ++len) {
      if (len > 22) return 1;
      int top = len - 1;  // 新しく入る問題 i + top のビット
      best.resize(1 << len, over);
      for (int s = 1 << top; s < (1 << len); ++s) {
        long long v = over;
        for (int t = 0; t < len; ++t)
          if (s >> t & 1) v = std::min(v, std::min(over, a[i + t] * best[s ^ (1 << t)] + b[i + t]));
        best[s] = v;
      }
      cost[i].push_back(best[(1 << len) - 1]);
      if (best[(1 << len) - 1] >= over) break;  // これより長い区間も X を超える
    }
  }
  // dp[j]: 先頭 j 問を k 日で解く最小の疲労。k を 1 つずつ増やす。
  std::vector<long long> dp(n + 1, over);
  dp[0] = 0;
  for (int k = 1; k <= n; ++k) {
    std::vector<long long> next(n + 1, over);
    for (int i = 0; i < n; ++i)
      if (dp[i] < over)
        for (int len = 1; len <= (int)cost[i].size(); ++len)
          next[i + len] = std::min(next[i + len], std::min(over, dp[i] + cost[i][len - 1]));
    dp = std::move(next);
    if (dp[n] <= x) {
      printf("%d %lld\n", k, dp[n]);
      return 0;
    }
  }
  return 1;
}
