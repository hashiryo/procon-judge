// abc179-c の愚直解。A と B を 1 から順に動かし、A B < N となる組を 1 つずつ数える
// (C = N - A B は 1 以上で 1 つに決まる)。商の列挙もディリクレ級数も素数上の和も使わないので、
// 提出とは別の考え方になる。O(N log N) なので、N = 10^6 でも速い。
#include <cstdio>

int main() {
  long long n;
  if (scanf("%lld", &n) != 1) return 1;
  long long count = 0;
  for (long long a = 1; a < n; ++a)
    for (long long b = 1; a * b < n; ++b) ++count;
  printf("%lld\n", count);
}
