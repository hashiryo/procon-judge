// abc276-g の愚直解。長さ i で最後の値が v の数列の数を、v = 0 から M まで持つ DP。
// 次の値 v は、v 以下で 3 で割った余りが v と違う値 u のあとに置ける。u ≡ v の和を余りごとの累積和で引く。
// 差を 3 で割った商と余りに分ける母関数も、疎な多項式の累乗も使わないので、提出とは別の考え方になる。O(NM)。
#include <cstdio>
#include <utility>
#include <vector>

using u64 = unsigned long long;
constexpr u64 MOD = 998244353;

int main() {
  long long n, m;
  if (scanf("%lld %lld", &n, &m) != 2) return 1;
  std::vector<u64> dp(m + 1, 1), next(m + 1);
  for (long long i = 1; i < n; ++i) {
    u64 all = 0, by[3] = {0, 0, 0};
    for (long long v = 0; v <= m; ++v) {
      all = (all + dp[v]) % MOD, by[v % 3] = (by[v % 3] + dp[v]) % MOD;
      next[v] = (all + MOD - by[v % 3]) % MOD;
    }
    std::swap(dp, next);
  }
  u64 sum = 0;
  for (u64 v : dp) sum = (sum + v) % MOD;
  printf("%llu\n", sum);
}
