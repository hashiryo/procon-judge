// abc154-e の愚直解。1 から N までの数を 1 つずつ見て、10 で割りながら 0 でない桁を数える。
// 桁ごとの状態を持つオートマトンの DP をしないので、Automaton とは別の考え方になる。
// O(N log N) なので、N が小さい (long long に収まる) ときだけ使う。
#include <cstdio>

int main() {
  long long n;
  int k;
  if (scanf("%lld %d", &n, &k) != 2) return 1;
  long long count = 0;
  for (long long i = 1; i <= n; ++i) {
    int nonzero = 0;
    for (long long x = i; x > 0; x /= 10)
      if (x % 10 != 0) ++nonzero;
    if (nonzero == k) ++count;
  }
  printf("%lld\n", count);
}
