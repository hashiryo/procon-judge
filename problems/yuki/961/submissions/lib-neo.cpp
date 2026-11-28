#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
// クエリ点を Mo の順に動かし、点を含む長方形の色の重複なし XOR を持つ。
// x を動かすときは、その x で始まる/終わる長方形のうち y が範囲内のものを足し引きし、
// y を動かすときは、x が範囲内のものを足し引きする。
// 境界の長方形は (範囲の下端, 上端, 色) を下端の昇順に並べて持ち、y (x) が下端より小さい
// ところで打ち切る。どの軸をブロックするかは、動く道のりと境界の個数の積で見積もって選ぶ。
struct Ev {
 int lo, hi, col;
};
int main() {
 int n;
 scanf("%d", &n);
 const int lim= 2 * n;
 vector<unsigned long long> h(n + 1);
 for(int i= 1; i <= n; ++i) scanf("%llu", &h[i]);
 vector<int> a(n), b(n), c(n), d(n), e(n);
 vector<int> wxs(lim, 0), wys(lim, 0);
 for(int i= 0; i < n; ++i) {
  scanf("%d %d %d %d %d", &a[i], &b[i], &c[i], &d[i], &e[i]);
  ++wxs[a[i]], ++wxs[c[i]], ++wys[b[i]], ++wys[d[i]];
 }
 int q;
 scanf("%d", &q);
 vector<int> px(q), py(q);
 int maxp= 0, maxq= 0;
 for(int i= 0; i < q; ++i) {
  scanf("%d %d", &px[i], &py[i]);
  if(maxp < px[i]) maxp= px[i];
  if(maxq < py[i]) maxq= py[i];
 }
 // x を軸 1 にするときの区間は [p + 1, q + offx)、y を軸 1 にするときは [q + 1, p + offy)。
 // 軸 1 の初期値 0 は座標 -1、軸 2 の初期値 0 は -off。
 const int offx= maxp + 1, offy= maxq + 1;
 vector<int> qx1(q), qx2(q), qy1(q), qy2(q);
 for(int i= 0; i < q; ++i) qx1[i]= px[i] + 1, qx2[i]= py[i] + offx, qy1[i]= py[i] + 1, qy2[i]= px[i] + offy;
 vector<long long> pwx(lim + 2, 0), pwy(lim + 2, 0);
 for(int v= 0; v < lim; ++v) pwx[v + 1]= pwx[v] + wxs[v], pwy[v + 1]= pwy[v] + wys[v];
 auto ranged= [](const vector<long long>& pw, int lo, int hi) { return pw[hi + 1] - pw[lo]; };
 // 軸 1 の座標 v (v >= 1) はもとの座標 v - 1、軸 2 の座標 r はもとの座標 r - off。
 auto est= [&](const vector<int>& ql, const vector<int>& qr, bool on_x, int maxr) -> long long {
  int bs= maxr / max(1, (int)sqrt((double)q));
  if(bs < 1) bs= 1;
  vector<int> ord(q);
  iota(ord.begin(), ord.end(), 0);
  sort(ord.begin(), ord.end(), [&](int i, int j) {
   const int bi= ql[i] / bs, bj= ql[j] / bs;
   if(bi != bj) return bi < bj;
   return (bi & 1) ? qr[i] > qr[j] : qr[i] < qr[j];
  });
  auto cost_l= [&](int from, int to) -> long long {
   if(from == to) return 0;
   const int lo= min(from, to), hi= max(from, to);  // 踏む軸 1 の座標は (lo, hi]
   return on_x ? ranged(pwx, lo, hi - 1) : ranged(pwy, lo, hi - 1);
  };
  auto cost_r= [&](int from, int to) -> long long {
   if(from == to) return 0;
   const int lo= min(from, to), hi= max(from, to);  // 踏む軸 2 の座標は (lo, hi]
   const int off= on_x ? offx : offy;
   const int s= max(lo + 1, off), t= hi;
   if(s > t) return 0;
   return on_x ? ranged(pwy, s - off, t - off) : ranged(pwx, s - off, t - off);
  };
  long long total= 0;
  int l= 0, r= 0;
  for(int id: ord) total+= cost_l(l, ql[id]) + cost_r(r, qr[id]), l= ql[id], r= qr[id];
  return total;
 };
 const bool block_on_x= est(qx1, qx2, true, offx + maxq) <= est(qy1, qy2, false, offy + maxp);
 // 境界ごとの長方形の一覧。x の境界は (b, d, 色)、y の境界は (a, c, 色)。
 vector<int> sxc(lim + 1, 0), exc(lim + 1, 0), syc(lim + 1, 0), eyc(lim + 1, 0);
 for(int i= 0; i < n; ++i) ++sxc[a[i]], ++exc[c[i]], ++syc[b[i]], ++eyc[d[i]];
 vector<int> sxo(lim + 2, 0), exo(lim + 2, 0), syo(lim + 2, 0), eyo(lim + 2, 0);
 for(int v= 0; v < lim; ++v) {
  sxo[v + 1]= sxo[v] + sxc[v], exo[v + 1]= exo[v] + exc[v];
  syo[v + 1]= syo[v] + syc[v], eyo[v + 1]= eyo[v] + eyc[v];
 }
 vector<Ev> sxl(sxo[lim]), exl(exo[lim]), syl(syo[lim]), eyl(eyo[lim]);
 vector<int> sp(sxo), ep(exo), yp(syo), yq(eyo);
 for(int i= 0; i < n; ++i) {
  sxl[sp[a[i]]++]= {b[i], d[i], e[i]}, exl[ep[c[i]]++]= {b[i], d[i], e[i]};
  syl[yp[b[i]]++]= {a[i], c[i], e[i]}, eyl[yq[d[i]]++]= {a[i], c[i], e[i]};
 }
 auto sort_ev= [&](vector<Ev>& ev, const vector<int>& off) {
  for(int v= 0; v < lim; ++v) {
   auto s= ev.begin() + off[v], t= ev.begin() + off[v + 1];
   sort(s, t, [](const Ev& x, const Ev& y) { return x.lo < y.lo; });
  }
 };
 sort_ev(sxl, sxo), sort_ev(exl, exo), sort_ev(syl, syo), sort_ev(eyl, eyo);
 Mo mo;
 if(block_on_x)
  for(int i= 0; i < q; ++i) mo.query(qx1[i], qx2[i]);
 else
  for(int i= 0; i < q; ++i) mo.query(qy1[i], qy2[i]);
 vector<unsigned long long> ans(q);
 vector<int> cnt(n + 1, 0);
 unsigned long long xors= 0;
 int curx= -1, cury= -1;
 if(block_on_x) cury= -offx;
 else curx= -offy;
 auto apply_color= [&](int col, int delta) {
  if(cnt[col] == 0) xors^= h[col];
  cnt[col]+= delta;
  if(cnt[col] == 0) xors^= h[col];
 };
 auto move_x_to= [&](int tx) {
  if(tx > curx) {
   do {
    if(++curx >= 0) {
     for(int i= sxo[curx]; i < sxo[curx + 1]; ++i) {
      const Ev& ev= sxl[i];
      if(ev.lo > cury) break;
      if(cury < ev.hi) apply_color(ev.col, 1);
     }
     for(int i= exo[curx]; i < exo[curx + 1]; ++i) {
      const Ev& ev= exl[i];
      if(ev.lo > cury) break;
      if(cury < ev.hi) apply_color(ev.col, -1);
     }
    }
   } while(curx < tx);
  } else {
   while(curx > tx) {
    for(int i= sxo[curx]; i < sxo[curx + 1]; ++i) {
     const Ev& ev= sxl[i];
     if(ev.lo > cury) break;
     if(cury < ev.hi) apply_color(ev.col, -1);
    }
    for(int i= exo[curx]; i < exo[curx + 1]; ++i) {
     const Ev& ev= exl[i];
     if(ev.lo > cury) break;
     if(cury < ev.hi) apply_color(ev.col, 1);
    }
    --curx;
   }
  }
 };
 auto move_y_to= [&](int ty) {
  if(ty > cury) {
   do {
    if(++cury >= 0) {
     for(int i= syo[cury]; i < syo[cury + 1]; ++i) {
      const Ev& ev= syl[i];
      if(ev.lo > curx) break;
      if(curx < ev.hi) apply_color(ev.col, 1);
     }
     for(int i= eyo[cury]; i < eyo[cury + 1]; ++i) {
      const Ev& ev= eyl[i];
      if(ev.lo > curx) break;
      if(curx < ev.hi) apply_color(ev.col, -1);
     }
    }
   } while(cury < ty);
  } else {
   while(cury > ty) {
    for(int i= syo[cury]; i < syo[cury + 1]; ++i) {
     const Ev& ev= syl[i];
     if(ev.lo > curx) break;
     if(curx < ev.hi) apply_color(ev.col, -1);
    }
    for(int i= eyo[cury]; i < eyo[cury + 1]; ++i) {
     const Ev& ev= eyl[i];
     if(ev.lo > curx) break;
     if(curx < ev.hi) apply_color(ev.col, 1);
    }
    --cury;
   }
  }
 };
 auto out= [&](int id) { ans[id]= xors; };
 if(block_on_x) {
  auto add_left= [&](int t) { move_x_to(t - 1); };
  auto add_right= [&](int t) { move_y_to(t + 1 - offx); };
  auto erase_left= [&](int t) { move_x_to(t); };
  auto erase_right= [&](int t) { move_y_to(t - offx); };
  mo.run(add_left, add_right, erase_left, erase_right, out);
 } else {
  auto add_left= [&](int t) { move_y_to(t - 1); };
  auto add_right= [&](int t) { move_x_to(t + 1 - offy); };
  auto erase_left= [&](int t) { move_y_to(t); };
  auto erase_right= [&](int t) { move_x_to(t - offy); };
  mo.run(add_left, add_right, erase_left, erase_right, out);
 }
 for(int i= 0; i < q; ++i) printf("%llu\n", ans[i]);
 return 0;
}
