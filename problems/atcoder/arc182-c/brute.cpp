// arc182-c の愚直解。長さ 1 から N までの列を深さ優先で全部たどり、積の素因数の指数を持ちながら、
// 列ごとに約数の個数 (指数 + 1 の積) を足す。部分集合の冪級数の累乗や割り算を使わないので、
// 提出とは別の考え方になる。M + M^2 + ... + M^N が小さい入力でだけ使う。
#include <cstdio>

using u64 = unsigned long long;
constexpr u64 P = 998244353;
constexpr int PRIMES[6] = {2, 3, 5, 7, 11, 13};
long long n;
int m, exponent[17][6], e[6];
u64 total = 0;

void walk(long long depth) {  // depth 個の要素を並べた列から、1 つ足していく
  if (depth == n) return;
  for (int a = 1; a <= m; ++a) {
    u64 divisors = 1;
    for (int j = 0; j < 6; ++j) e[j] += exponent[a][j], divisors *= e[j] + 1;
    total = (total + divisors) % P;
    walk(depth + 1);
    for (int j = 0; j < 6; ++j) e[j] -= exponent[a][j];
  }
}

int main() {
  if (scanf("%lld %d", &n, &m) != 2) return 1;
  for (int a = 1; a <= 16; ++a)
    for (int j = 0, x = a; j < 6; ++j)
      while (x % PRIMES[j] == 0) x /= PRIMES[j], ++exponent[a][j];
  walk(0);
  printf("%llu\n", total);
}
