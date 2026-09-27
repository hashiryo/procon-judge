// agc021-a の愚直解。1 から N までの整数を全部たどって、桁和の最大をとる。
// 桁ごとの状態を持つ Automaton の DP を使わないので、提出とは別の考え方になる。O(N log N) なので、
// 小さい N でだけ使う。
#include <cstdio>

int main() {
  long long n;
  if (scanf("%lld", &n) != 1) return 1;
  int best = 0;
  for (long long x = 1; x <= n; ++x) {
    int s = 0;
    for (long long y = x; y > 0; y /= 10) s += y % 10;
    if (s > best) best = s;
  }
  printf("%d\n", best);
}
