// 書き比べ: lib-neo と同じで、k 進の段の残りのループを clang がベクトル化しないようにした NeoLibrary の zeta.hpp の写し
// (_shared/sps/zeta_nv.hpp) を読む。x64-clang で lib-neo が x64-gcc の 1.3 倍から 2.3 倍かかる分を見る。比べ終えたら消す。
// NeoLibrary の sps::subset_zeta(f, 10) (neo/set_power_series/zeta.hpp、k 進の桁ごとの累積和) で、値ごとの個数を桁ごとに x 以下の
// 個数にする。A_i ごとに c[999999 - A_i] を足し、自分自身との組 (どの桁も 4 以下) を引いて 2 で割る。
#include <cstdio>
#include <vector>
#include "_shared/sps/zeta_nv.hpp"
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
