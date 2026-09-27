// abc172-d の愚直解。約数 d を 1 から N まで動かし、d の倍数 K のそれぞれに K を足す
// (K は約数の数 f(K) 回だけ足されるので、和が K f(K) の和になる)。約数の数の表もディリクレ級数も
// 商の列挙も素数上の和も使わないので、提出とは別の考え方になる。O(N log N) なので、小さい N でだけ使う。
#include <cstdio>

int main() {
  long long n;
  if (scanf("%lld", &n) != 1) return 1;
  long long sum = 0;
  for (long long d = 1; d <= n; ++d)
    for (long long k = d; k <= n; k += d) sum += k;
  printf("%lld\n", sum);
}
