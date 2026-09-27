// abc121-d の愚直解。A から B まで 1 つずつ排他的論理和を取る。
// ビットごとのオートマトンで数えないので、提出とは別の考え方になる。B - A が小さいときだけ使う。
#include <cstdio>

int main() {
  long long a, b;
  if (scanf("%lld %lld", &a, &b) != 2) return 1;
  long long x = 0;
  for (long long v = a; v <= b; ++v) x ^= v;
  printf("%lld\n", x);
}
