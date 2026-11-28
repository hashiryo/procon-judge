// pakencamp 2022 day1 N (Paken Machine)。モノイド作用の一般版で解く。
//
// 色は 0 = 赤、1 = 青、2 = 黄で、押すと色 c の写像 f_c(x) = (A_c x + B_c) mod P を作用させて c + 1 に移る。
// 3 回押すと色は元に戻るので、色 c から始める 3 回ぶんの写像 g_c = f_{c+2} o f_{c+1} o f_c を作る。
// n = 3q + r とすると、r 回押したあとの状態 x_r から g_r を q 回作用させた値が表示になる。
// 各 r で g_r^q(x_r) = t を離散対数で解き (q < P)、n = 3q + r の最小を取る。
#include "pj.hpp"
#include "neo/algo/compute/DiscreteLogAction.hpp"
using namespace std;
using Aff = array<long long, 2>;

signed main() {
  int T;
  must_scan(scanf("%d", &T), 1);
  while (T--) {
    long long s, t, P;
    must_scan(scanf("%lld %lld %lld", &s, &t, &P), 3);
    Aff f[3];
    for (int c = 0; c < 3; ++c) must_scan(scanf("%lld %lld", &f[c][0], &f[c][1]), 2);
    auto apply = [P](Aff m, long long x) { return (m[0] * x + m[1]) % P; };
    auto compose = [P](Aff m1, Aff m2) {
      return Aff{m1[0] * m2[0] % P, (m1[0] * m2[1] + m1[1]) % P};
    };
    Aff g[3];
    for (int c = 0; c < 3; ++c) g[c] = compose(f[(c + 2) % 3], compose(f[(c + 1) % 3], f[c]));
    auto hash = [](long long x) { return (unsigned)x; };
    long long best = -1, x = s;
    for (int r = 0; r < 3; ++r) {
      if (r > 0) x = apply(f[r - 1], x);
      DiscreteLogAction dlog(apply, compose, hash, P);
      const long long q = dlog(g[r], x, t, P);
      if (q >= 0) {
        const long long n = 3 * q + r;
        if (best < 0 || n < best) best = n;
      }
    }
    printf("%lld\n", best);
  }
  return 0;
}
