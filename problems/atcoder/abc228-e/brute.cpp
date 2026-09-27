// abc228-e の愚直解。x = M から始めて x = x^K mod P を N 回くり返す。
// 指数 K^N を P - 1 で縮めないので、ModInt_Exp とは別の考え方になる。N が小さいときだけ使う。
#include <cstdio>

using u64 = unsigned long long;
constexpr u64 P = 998244353;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int main() {
  u64 n, k, m;
  if (scanf("%llu %llu %llu", &n, &k, &m) != 3) return 1;
  u64 x = m % P;
  for (u64 i = 0; i < n; ++i) x = pow_mod(x, k);
  printf("%llu\n", x);
}
