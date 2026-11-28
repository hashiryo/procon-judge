// gen.py の seed 1000 以上の小さい入力を、色と表示をそのまま辿って解く。crosscheck 用 (提出ではない)。
#include "pj.hpp"

signed main() {
  int T;
  must_scan(scanf("%d", &T), 1);
  while (T--) {
    long long s, t, P;
    must_scan(scanf("%lld %lld %lld", &s, &t, &P), 3);
    array<long long, 2> f[3];
    for (int c = 0; c < 3; ++c) must_scan(scanf("%lld %lld", &f[c][0], &f[c][1]), 2);
    long long x = s, ans = -1;
    int c = 0;
    for (long long n = 0; n < 3 * P; ++n) {
      if (x == t) {
        ans = n;
        break;
      }
      x = (f[c][0] * x + f[c][1]) % P;
      c = (c + 1) % 3;
    }
    printf("%lld\n", ans);
  }
  return 0;
}
