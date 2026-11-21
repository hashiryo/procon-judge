// NeoLibrary の sps::subset_zeta(f, 10) (neo/base/core/set_fps/zeta.hpp、k 進の桁ごとの累積和) で、値ごとの個数を桁ごとに x 以下の
// 個数にする。A_i ごとに c[999999 - A_i] を足し、自分自身との組 (どの桁も 4 以下) を引いて 2 で割る。
#include <cstdio>
#include <vector>
#include "neo/base/core/set_fps/zeta.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<int> a(n);
 std::vector<long long> c(1000000, 0);
 for(auto& v: a) {
  if(scanf("%d", &v) != 1) return 1;
  ++c[v];
 }
 sps::subset_zeta(c, 10);
 long long ans= 0;
 for(int v: a) {
  ans+= c[999999 - v];
  bool self= true;
  for(int x= v; x; x/= 10) self&= x % 10 <= 4;
  ans-= self;
 }
 printf("%lld\n", ans / 2);
}
