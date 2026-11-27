// 実験中の多点評価 (部分積木と余りの木、_shared/ntt/multipoint.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <cstdio>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/multipoint.hpp"
#include "_shared/ntt/ntt_avx2.hpp"
using namespace std;
signed main() {
 int n, m;
 if (scanf("%d %d", &n, &m) != 2) return 1;
 using Z= ZMod<998244353>;
 vector<Z> f(n), x(m);
 for (int i= 0; i < n; ++i) {
  unsigned t;
  if (scanf("%u", &t) != 1) return 1;
  f[i]= Z::raw(t);
 }
 for (int i= 0; i < m; ++i) {
  unsigned t;
  if (scanf("%u", &t) != 1) return 1;
  x[i]= Z::raw(t);
 }
 const vector<Z> r= multipoint_eval<NttAvx2>(f, x);
 for (int i= 0; i < m; ++i) printf("%u%c", r[i].val(), i + 1 == m ? '\n' : ' ');
 return 0;
}
