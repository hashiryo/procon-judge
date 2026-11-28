// https://judge.yosupo.jp/problem/static_range_count_distinct
// 既にある base の harness の問題 (per-query で答える wavelet matrix の比較) とは別に、
// クエリを全部読んでから並べ替える Mo を測る raw の問題として置く。
#include <algorithm>
#include <cstdio>
#include <vector>
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
 vector<int> cnt(xs.size(), 0), ans(q);
 int distinct= 0;
 auto add= [&](int i) {
  if(cnt[a[i]]++ == 0) ++distinct;
 };
 auto erase= [&](int i) {
  if(--cnt[a[i]] == 0) --distinct;
 };
 auto out= [&](int id) { ans[id]= distinct; };
 mo.run(add, erase, out);
 for(int i= 0; i < q; ++i) printf("%d\n", ans[i]);
}
