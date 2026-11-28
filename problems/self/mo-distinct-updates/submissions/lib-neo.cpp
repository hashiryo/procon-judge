// NeoLibrary の MoWithUpdates を使う提出。点更新つきの区間 distinct を 3D Mo で解く。
// 値は座標圧縮し、クエリは 0-indexed の半開区間 [l, r) と、その操作より前の更新の数 t にする。
// 更新は (位置, 古い値, 新しい値) に直して持つ。apply と undo はそのときの区間 [l, r) を受け取り、
// 更新の位置が区間に入るときだけ distinct を直す。
#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/MoWithUpdates.hpp"
using namespace std;

signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 vector<int> a(n), vals;
 vals.reserve(n + q);
 for(int i= 0; i < n; ++i) cin >> a[i], vals.push_back(a[i]);
 struct Op { int type, x, y; };
 vector<Op> ops(q);
 for(int i= 0; i < q; ++i) {
  cin >> ops[i].type >> ops[i].x >> ops[i].y;
  if(ops[i].type == 0) vals.push_back(ops[i].y);
 }
 sort(vals.begin(), vals.end());
 vals.erase(unique(vals.begin(), vals.end()), vals.end());
 auto comp= [&](int v) { return int(lower_bound(vals.begin(), vals.end(), v) - vals.begin()); };
 for(int& x: a) x= comp(x);
 // 操作の列を、更新の列 (位置, 古い値, 新しい値) と、クエリの列 (l, r, t) に分ける。
 vector<int> upos, uold, unew, ql, qr, qt;
 upos.reserve(q), uold.reserve(q), unew.reserve(q);
 ql.reserve(q), qr.reserve(q), qt.reserve(q);
 {
  vector<int> cur= a;
  for(const Op& op: ops) {
   if(op.type == 0) {
    int p= op.x - 1, w= comp(op.y);
    upos.push_back(p), uold.push_back(cur[p]), unew.push_back(w);
    cur[p]= w;
   } else {
    ql.push_back(op.x - 1), qr.push_back(op.y), qt.push_back((int)upos.size());
   }
  }
 }
 MoWithUpdates mo;
 for(int i= 0; i < (int)ql.size(); ++i) mo.query(ql[i], qr[i], qt[i]);
 vector<int> cnt(vals.size(), 0), ans(ql.size());
 int distinct= 0;
 vector<int> cur= a;
 auto add= [&](int i) {
  if(cnt[cur[i]]++ == 0) ++distinct;
 };
 auto erase= [&](int i) {
  if(--cnt[cur[i]] == 0) --distinct;
 };
 auto apply= [&](int k, int l, int r) {
  int p= upos[k], o= uold[k], w= unew[k];
  cur[p]= w;
  if(l <= p && p < r) {
   if(--cnt[o] == 0) --distinct;
   if(cnt[w]++ == 0) ++distinct;
  }
 };
 auto undo= [&](int k, int l, int r) {
  int p= upos[k], o= uold[k], w= unew[k];
  cur[p]= o;
  if(l <= p && p < r) {
   if(--cnt[w] == 0) --distinct;
   if(cnt[o]++ == 0) ++distinct;
  }
 };
 auto out= [&](int id) { ans[id]= distinct; };
 mo.run(add, add, erase, erase, apply, undo, out);
 for(int i= 0; i < (int)ans.size(); ++i) cout << ans[i] << '\n';
}
