// 実験中の補間 (部分積木と P' の多点評価、_shared/ntt/multipoint.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <cstdio>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/multipoint.hpp"
#include "_shared/ntt/ntt_avx2.hpp"
using namespace std;
signed main() {
 int n;
 if (scanf("%d", &n) != 1) return 1;
 using Z= ZMod<998244353>;
 vector<Z> x(n), y(n);
 for (int i= 0; i < n; ++i) {
  unsigned t;
  if (scanf("%u", &t) != 1) return 1;
  x[i]= Z::raw(t);
 }
 for (int i= 0; i < n; ++i) {
  unsigned t;
  if (scanf("%u", &t) != 1) return 1;
  y[i]= Z::raw(t);
 }
 const vector<Z> f= interpolate<NttAvx2>(x, y);
 for (int i= 0; i < n; ++i) printf("%u%c", f[i].val(), i + 1 == n ? '\n' : ' ');
 return 0;
}
