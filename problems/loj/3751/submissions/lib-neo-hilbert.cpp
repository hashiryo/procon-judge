#include <iostream>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<int> a(n);
 for(auto& x: a) cin >> x;
 int m;
 cin >> m;
 Mo mo;
 for(int i= 0; i < m; ++i) {
  int l, r;
  cin >> l >> r, --l;
  mo.query(l, r);
 }
 vector<int> cnt(1000001, 0), ans(m);
 int distinct= 0;
 auto add= [&](int i) {
  if(cnt[a[i]]++ == 0) ++distinct;
 };
 auto erase= [&](int i) {
  if(--cnt[a[i]] == 0) --distinct;
 };
 auto out= [&](int id) { ans[id]= distinct; };
 mo.run_hilbert(add, erase, out);
 for(int i= 0; i < m; ++i) cout << ans[i] << '\n';
}
