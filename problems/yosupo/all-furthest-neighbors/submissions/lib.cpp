#include <cstdio>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
using namespace geo;
using ll= long long;
// 各行の最大が単調な行列で、行ごとの最大を分割統治で求める。
template <class F> static vector<int> monotone_maxima(int h, int w, const F& value) {
 vector<int> max_col(h);
 auto dfs= [&](auto&& self, int row_l, int row_r, int col_l, int col_r) -> void {
  if(row_l == row_r) return;
  int row= (row_l + row_r) / 2, best= col_l;
  for(int col= col_l + 1; col < col_r; ++col)
   if(value(row, best) < value(row, col)) best= col;
  max_col[row]= best;
  self(self, row_l, row, col_l, best + 1);
  self(self, row + 1, row_r, best, col_r);
 };
 return dfs(dfs, 0, h, 0, w), max_col;
}
signed main() {
 int T;
 scanf("%d", &T);
 while(T--) {
  int n;
  scanf("%d", &n);
  vector<Point<ll>> p(n);
  for(auto& q: p) scanf("%lld %lld", &q.x, &q.y);
  auto value= [&](int i, int j) -> make_long_t<ll> {
   if(j <= i) return j - i;
   if(j >= i + n) return -1;
   return dist2(p[i], p[j % n]);
  };
  vector<int> ans= monotone_maxima(n, 2 * n - 1, value);
  for(int i= 0; i < n; ++i) printf(i ? " %d" : "%d", ans[i] % n);
  puts("");
 }
 return 0;
}
