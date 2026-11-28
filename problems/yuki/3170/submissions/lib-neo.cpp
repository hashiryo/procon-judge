// yuki 3170。2x2 行列 A^n = B (mod P) の最小の n を、モノイド作用の一般版で直接解く。
// 軌道の長さは P^2 未満 (A が正則なら位数は P^2 - 1 以下、特異なら I, A, A^2, ... の周期は P - 1 以下) なので、
// 探索の上限は P^2 でよい。
#include <array>
#include "pj.hpp"
#include "neo/algo/compute/DiscreteLogAction.hpp"
using namespace std;
using Mat = array<long long, 4>;

signed main() {
  int T;
  must_scan(scanf("%d", &T), 1);
  while (T--) {
    long long P;
    must_scan(scanf("%lld", &P), 1);
    Mat A, B;
    for (auto &v : A) must_scan(scanf("%lld", &v), 1);
    for (auto &v : B) must_scan(scanf("%lld", &v), 1);
    auto mul = [P](Mat f, Mat g) {
      Mat r;
      r[0] = (f[0] * g[0] + f[1] * g[2]) % P;
      r[1] = (f[0] * g[1] + f[1] * g[3]) % P;
      r[2] = (f[2] * g[0] + f[3] * g[2]) % P;
      r[3] = (f[2] * g[1] + f[3] * g[3]) % P;
      return r;
    };
    auto hash = [](Mat v) { return (unsigned)v[0]; };
    DiscreteLogAction dlog(mul, mul, hash, P * P);
    const long long ans = dlog(A, Mat{1 % P, 0, 0, 1 % P}, B, P * P);
    printf("%lld\n", ans);
  }
  return 0;
}
