// abc208-f の愚直解。定義のとおり f(n, m) = f(n - 1, m) + f(n, m - 1) の表を、n = 0 から N まで 1 行ずつ作る。
// 多項式として補間しないので、sample_points_shift とは別の考え方になる。O(N (M + log K)) なので、
// N が小さいときだけ使う。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
constexpr u64 P = 1000000007;

u64 pow_mod(u64 x, u64 e) {
  u64 r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int main() {
  u64 n, m, k;
  if (scanf("%llu %llu %llu", &n, &m, &k) != 3) return 1;
  std::vector<u64> row(m + 1, 0);  // row[j] = f(i, j)。i = 0 の行は全部 0
  for (u64 i = 1; i <= n; ++i) {
    row[0] = pow_mod(i, k);
    for (u64 j = 1; j <= m; ++j) row[j] = (row[j] + row[j - 1]) % P;  // f(i - 1, j) + f(i, j - 1)
  }
  printf("%llu\n", row[m]);
}
