// 実験中の標本点のずらし (_shared/ntt/sample_shift.hpp) を、実験中の単独の NTT の核 (3 本目、AVX2) で回す。
#include <cstdio>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/sample_shift.hpp"
#include "_shared/ntt/ntt_avx2.hpp"
using namespace std;
signed main() {
 int n, m;
 unsigned cv;
 if (scanf("%d %d %u", &n, &m, &cv) != 3) return 1;
 using Z= ZMod<998244353>;
 const Z c= Z::raw(cv);
 vector<Z> y(n);
 for (int i= 0; i < n; ++i) {
  unsigned t;
  if (scanf("%u", &t) != 1) return 1;
  y[i]= Z::raw(t);
 }
 const vector<Z> r= sample_points_shift<NttAvx2>(y, c, m);
 for (int i= 0; i < m; ++i) printf("%u%c", r[i].val(), i + 1 == m ? '\n' : ' ');
 return 0;
}
