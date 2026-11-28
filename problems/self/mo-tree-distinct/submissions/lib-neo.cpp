#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/MoOnTree.hpp"
using namespace std;
signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 vector<int> c(n);
 for (auto &x : c) cin >> x;
 vector<int> xs = c;
 sort(xs.begin(), xs.end()), xs.erase(unique(xs.begin(), xs.end()), xs.end());
 for (auto &x : c) x = lower_bound(xs.begin(), xs.end(), x) - xs.begin();
 MoOnTree mo(n);
 for (int i = 0; i < n - 1; ++i) {
  int u, v;
  cin >> u >> v, --u, --v;
  mo.add_edge(u, v);
 }
 for (int i = 0; i < q; ++i) {
  int u, v;
  cin >> u >> v, --u, --v;
  mo.query(u, v);
 }
 mo.build(0);
 vector<int> cnt(xs.size(), 0), ans(q);
 int distinct = 0;
 auto toggle = [&](int v, bool entering) {
  const int x = c[v];
  if (entering) {
   if (cnt[x]++ == 0) ++distinct;
  } else {
   if (--cnt[x] == 0) --distinct;
  }
 };
 auto out = [&](int id) { ans[id] = distinct; };
 mo.run(toggle, out);
 for (int i = 0; i < q; ++i) cout << ans[i] << '\n';
}
