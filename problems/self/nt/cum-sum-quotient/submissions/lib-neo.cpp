#include <cassert>
#include <random>
#include <vector>
#include "neo/base/core/nt/quotient.hpp"

using u64= unsigned long long;
using i64= long long;

static bool in_S(u64 n, u64 N) { return 1 <= n && n <= N && N / (N / n) == n; }

signed main() {
 std::mt19937_64 rng(20261128);
 for(u64 N= 1; N <= 200; ++N) {
  std::vector<i64> pref(N + 1, 0);
  CumSumQuotient<i64> cq(N), cq2(N);
  for(int step= 0; step < 40; ++step) {
   const u64 i= 1 + rng() % N, j= 1 + rng() % N;
   const i64 v= i64(rng() % 201) - 100, w= i64(rng() % 201) - 100;
   for(u64 k= i; k <= N; ++k) pref[k]+= v;
   cq.add(i, v), cq2.add(j, w);
   for(u64 n= 1; n <= N; ++n)
    if(in_S(n, N)) {
     assert(cq(n) == pref[n]);
     assert(cq.sum(n) == pref[n]);
    }
   assert(cq.sum() == pref[N]);
   const auto sum2= cq + cq2, dif= cq - cq2, neg= -cq, sc= cq * 3;
   for(u64 n= 1; n <= N; ++n)
    if(in_S(n, N)) {
     assert(sum2(n) == cq(n) + cq2(n));
     assert(dif(n) == cq(n) - cq2(n));
     assert(neg(n) == -cq(n));
     assert(sc(n) == 3 * cq(n));
    }
  }
 }
 return 0;
}
