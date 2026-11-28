// 期待出力を作る参照実装。点更新つきの区間 distinct を 3D Mo (時間軸つき Mo's algorithm) で解く。
// ライブラリを include しない。
//
// 値は先に座標圧縮する (初期値と、更新で書き込む値の両方)。各クエリは (l, r, t) になる。l と r は
// 0-indexed の半開区間 [l, r) で、t はそのクエリより前の更新の数。更新の並びは、操作の列を前から
// たどって (位置, そのときの古い値, 新しい値) に直す。
//
// 並べ方は l のブロック、r のブロック、t の順。ブロックの幅は n^(2/3)。区間の端点を動かしてから
// 時刻を動かし、更新の位置がそのときの区間 [l, r) に入るときだけ distinct を直す。
// 愚直解 (brute.cpp) と小さい入力で突き合わせる (pj testdata crosscheck)。
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <vector>
using namespace std;

int main() {
 int n, q;
 if(scanf("%d %d", &n, &q) != 2) return 1;
 vector<int> a(n), vals;
 vals.reserve(n + q);
 for(int i= 0; i < n; ++i) scanf("%d", &a[i]), vals.push_back(a[i]);
 struct Op { int type, x, y; };
 vector<Op> ops(q);
 for(int i= 0; i < q; ++i) {
  if(scanf("%d %d %d", &ops[i].type, &ops[i].x, &ops[i].y) != 3) return 1;
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
 const int m= (int)ql.size();
 vector<int> ans(m);
 if(m > 0) {
  int bs= 1;
  if(n > 1) bs= (int)pow((double)n, 2.0 / 3.0);
  if(bs < 1) bs= 1;
  vector<int> ord(m), lb(m), rb(m);
  iota(ord.begin(), ord.end(), 0);
  for(int i= 0; i < m; ++i) lb[i]= ql[i] / bs, rb[i]= qr[i] / bs;
  sort(ord.begin(), ord.end(), [&](int x, int y) {
   if(lb[x] != lb[y]) return lb[x] < lb[y];
   if(rb[x] != rb[y]) return (lb[x] & 1) ? rb[x] > rb[y] : rb[x] < rb[y];
   return (rb[x] & 1) ? qt[x] > qt[y] : qt[x] < qt[y];
  });
  vector<int> cnt(vals.size(), 0);
  cur= a;
  int distinct= 0, l= 0, r= 0, t= 0;
  auto add= [&](int i) {
   if(cnt[cur[i]]++ == 0) ++distinct;
  };
  auto erase= [&](int i) {
   if(--cnt[cur[i]] == 0) --distinct;
  };
  for(int id: ord) {
   while(l > ql[id]) add(--l);
   while(r < qr[id]) add(r++);
   while(l < ql[id]) erase(l++);
   while(r > qr[id]) erase(--r);
   while(t < qt[id]) {
    int p= upos[t], o= uold[t], w= unew[t];
    cur[p]= w;
    if(l <= p && p < r) {
     if(--cnt[o] == 0) --distinct;
     if(cnt[w]++ == 0) ++distinct;
    }
    ++t;
   }
   while(t > qt[id]) {
    --t;
    int p= upos[t], o= uold[t], w= unew[t];
    cur[p]= o;
    if(l <= p && p < r) {
     if(--cnt[w] == 0) --distinct;
     if(cnt[o]++ == 0) ++distinct;
    }
   }
   ans[id]= distinct;
  }
 }
 for(int i= 0; i < m; ++i) printf("%d\n", ans[i]);
 return 0;
}
