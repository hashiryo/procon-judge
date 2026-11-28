// gen.py の seed 1000 以上の小さい入力を、1, z, z^2, ... を辿って解く。crosscheck 用 (提出ではない)。
#include "pj.hpp"

signed main() {
  long long A, B, C, D, p;
  must_scan(scanf("%lld %lld %lld %lld %lld", &A, &B, &C, &D, &p), 5);
  long long zx = A % p, zy = B % p;
  long long x = 1 % p, y = 0;
  long long ans = -1;
  for (long long k = 0; k <= p * p; ++k) {
    if (x == C % p && y == D % p) {
      ans = k;
      break;
    }
    long long nx = (x * zx - y * zy) % p, ny = (x * zy + y * zx) % p;
    if (nx < 0) nx += p;
    x = nx, y = ny;
  }
  printf("%lld\n", ans);
  return 0;
}
