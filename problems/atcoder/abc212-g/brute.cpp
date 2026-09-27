// abc212-g の愚直解。x ごとに x^1, x^2, ..., x^(P-1) を順にかけて求め、出てきた y に印を付けて数える。
// 位数ごとに φ(d) 個あるという数え方も、P - 1 の素因数分解もしないので、提出とは別の考え方になる。
// O(P^2) なので、小さい P でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long p;
  if (scanf("%lld", &p) != 1) return 1;
  std::vector<long long> seen(p, -1);
  long long count = 0;
  for (long long x = 0; x < p; ++x) {
    long long y = 1;
    for (long long n = 1; n <= p - 1; ++n) {
      y = y * x % p;
      if (seen[y] != x) seen[y] = x, ++count;
    }
  }
  printf("%lld\n", count % 998244353);
}
