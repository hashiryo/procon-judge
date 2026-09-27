// abc162-e の愚直解。g = Σ_{d | g} φ(d) を使い、答えを Σ_{d=1}^{K} φ(d) ⌊K/d⌋^N として求める
// (d が全部の A_i を割る列は ⌊K/d⌋^N 個)。φ(d) は d ごとに試し割りで素因数を見つけて求める。
// 倍数の上でメビウス変換をしないので、提出とは別の考え方になる。O(K √K) なので制約いっぱいでも解ける。
#include <cstdio>

using u64 = unsigned long long;
constexpr u64 P = 1000000007;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

u64 phi(u64 n) {
  u64 r = n;
  for (u64 p = 2; p * p <= n; ++p)
    if (n % p == 0) {
      while (n % p == 0) n /= p;
      r = r / p * (p - 1);
    }
  if (n > 1) r = r / n * (n - 1);
  return r;
}

int main() {
  u64 n, k;
  if (scanf("%llu %llu", &n, &k) != 2) return 1;
  u64 ans = 0;
  for (u64 d = 1; d <= k; ++d) ans = (ans + phi(d) % P * pow_mod(k / d, n)) % P;
  printf("%llu\n", ans);
}
