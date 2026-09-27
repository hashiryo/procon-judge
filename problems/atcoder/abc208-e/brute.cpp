// abc208-e の愚直解。1 から N まで、数ごとに各桁の積を求めて K 以下のものを数える。
// 桁を読むオートマトンを作らないので、提出とは別の考え方になる。N が小さいときだけ使う。
#include <cstdio>

int main() {
  long long n, k;
  if (scanf("%lld %lld", &n, &k) != 2) return 1;
  long long count = 0;
  for (long long x = 1; x <= n; ++x) {
    long long product = 1;
    for (long long y = x; y > 0; y /= 10) product *= y % 10;
    if (product <= k) ++count;
  }
  printf("%lld\n", count);
}
