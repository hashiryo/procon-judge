// https://judge.yosupo.jp/problem/static_range_inversions_query
// Mo で区間を動かし、値の出現数を BIT に持って、足す側の値より大きい (小さい) 個数を足し引きする。
#include <algorithm>
#include <cstdio>
#include <vector>
#include "neo/base/ds/range/Fenwick.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"
#include "neo/base/engine/query/Mo.hpp"
using namespace std;

signed main() {
 int n, q;
 scanf("%d %d", &n, &q);
 vector<int> a(n);
 for(auto& x: a) scanf("%d", &x);
 // 座標圧縮
 vector<int> xs(a);
 sort(xs.begin(), xs.end());
 xs.erase(unique(xs.begin(), xs.end()), xs.end());
 for(auto& x: a) x= (int)(lower_bound(xs.begin(), xs.end(), x) - xs.begin());
 Mo mo;
 for(int i= 0; i < q; ++i) {
  int l, r;
  scanf("%d %d", &l, &r);
  mo.query(l, r);
 }
 Fenwick<monoid::Sum<int>> fw((int)xs.size());
 long long inv= 0, total= 0;
 auto add_l= [&](int i) {
  int x= a[i];
  inv+= fw.prod(x), fw.add(x, 1), ++total;
 };
 auto add_r= [&](int i) {
  int x= a[i];
  inv+= total - fw.prod(x + 1), fw.add(x, 1), ++total;
 };
 auto erase_l= [&](int i) {
  int x= a[i];
  inv-= fw.prod(x), fw.add(x, -1), --total;
 };
 auto erase_r= [&](int i) {
  int x= a[i];
  inv-= total - fw.prod(x + 1), fw.add(x, -1), --total;
 };
 vector<long long> ans(q);
 auto out= [&](int id) { ans[id]= inv; };
 mo.run(add_l, add_r, erase_l, erase_r, out);
 for(int i= 0; i < q; ++i) printf("%lld\n", ans[i]);
}
