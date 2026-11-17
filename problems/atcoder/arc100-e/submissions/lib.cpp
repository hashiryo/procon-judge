// 今の Library の sps::subset_zeta (mylib/algebra/set_power_series.hpp)。値の型の += で畳むので、上位 2 個の併合を += に書いた型を渡す。
#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "mylib/algebra/set_power_series.hpp"
struct Top2 {
 int a= 0, b= 0;  // 大きい順の 2 個 (A_i >= 1 なので、無い値は 0)
 Top2& operator+=(const Top2& o) {
  if(a < o.a) b= std::max(a, o.b), a= o.a;
  else b= std::max(b, o.a);
  return *this;
 }
};
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 const int m= 1 << n;
 std::vector<Top2> f(m);
 for(auto& p: f)
  if(scanf("%d", &p.a) != 1) return 1;
 sps::subset_zeta(f);
 std::string out;
 out.reserve(size_t(m) * 12);
 long long best= 0;
 for(int k= 1; k < m; ++k) {
  best= std::max(best, (long long)f[k].a + f[k].b);
  out+= std::to_string(best), out+= '\n';
 }
 fwrite(out.data(), 1, out.size(), stdout);
}
