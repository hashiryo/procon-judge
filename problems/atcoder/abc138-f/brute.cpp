// abc138-f の愚直解。L ≤ x ≤ y ≤ R の組を全部たどって、y % x == (y ^ x) になるものを数える。
// ビットごとの状態を持つ Automaton も、最上位のビットと包含への言い換えも使わないので、提出とは別の考え方になる。
// O((R - L)^2) なので、幅の狭い区間でだけ使う。
#include <cstdio>

int main() {
  unsigned long long l, r;
  if (scanf("%llu %llu", &l, &r) != 2) return 1;
  unsigned long long count = 0;
  for (unsigned long long x = l; x <= r; ++x)
    for (unsigned long long y = x; y <= r; ++y)
      if (y % x == (y ^ x)) ++count;
  printf("%llu\n", count % 1000000007);
}
