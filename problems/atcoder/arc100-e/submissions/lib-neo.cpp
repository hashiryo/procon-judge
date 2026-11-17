// NeoLibrary の sps::subset_zeta<M> (neo/set_power_series/zeta.hpp) に、値の上位 2 個を持つモノイドの構造体を渡す。
// f[S] は S の部分集合の添字の値の上位 2 個になり、K ごとの答えは K 以下の S での 2 個の和の最大。
#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "neo/set_power_series/zeta.hpp"
struct Top2 {
 using T= std::pair<int, int>;  // 大きい順の 2 個 (A_i >= 1 なので、無い値は 0)
 static T ti() { return {0, 0}; }
 static T op(T x, T y) {
  if(x.first < y.first) std::swap(x, y);
  return {x.first, std::max(x.second, y.first)};
 }
};
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 const int m= 1 << n;
 std::vector<Top2::T> f(m);
 for(auto& p: f)
  if(scanf("%d", &p.first) != 1) return 1;
 sps::subset_zeta<Top2>(f);
 std::string out;
 out.reserve(size_t(m) * 12);
 long long best= 0;
 for(int k= 1; k < m; ++k) {
  best= std::max(best, (long long)f[k].first + f[k].second);
  out+= std::to_string(best), out+= '\n';
 }
 fwrite(out.data(), 1, out.size(), stdout);
}
