// NeoLibrary の sps::exp に MaxPlus<long long> を渡す (neo/base/core/set_fps/exp.hpp)。塊の得点 w[T] の exp の全体の値が答え
// (集合を塊に分けて、塊の得点の和の最大)。添字の下の bit を AVX2 のレーンに置く 3^n の核 (semiring_simd.hpp) に回る。
#include <cstdio>
#include <vector>
#include "neo/base/core/algebra/semiring.hpp"
#include "neo/base/core/set_fps/exp.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<long long> a(n * n);
 for(auto& v: a)
  if(scanf("%lld", &v) != 1) return 1;
 const int N= 1 << n;
 std::vector<MaxPlus<long long>> w(N, MaxPlus<long long>(0));
 for(int s= 1; s < N; ++s) {
  const int hi= 31 - __builtin_clz(s), rest= s ^ (1 << hi);
  long long x= w[rest].x;
  for(int j= 0; j < hi; ++j)
   if(rest >> j & 1) x+= a[hi * n + j];
  w[s]= MaxPlus<long long>(x);
 }
 printf("%lld\n", sps::exp(w)[N - 1].x);
}
