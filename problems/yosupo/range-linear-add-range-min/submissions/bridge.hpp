#pragma once
#include "pj.hpp"
#include <bit>

// 点 (i, a_i) の下側の凸包を、節ごとに左右の子の凸包を結ぶ橋 (共通の接線の 2 点) だけで持つ二分木
// (Overmars と van Leeuwen の形)。Library Checker の想定解と同じ作り。
//
// 区間に b i + c を足すのは、覆う節に作用 (b, c) を積むだけで、その節より下の橋は変わらない (1 次関数を足しても
// 点どうしの向きは変わらない)。端の道の節は、左右の子の凸包を橋から橋へ下りて接線を探し直す (1 節で O(log N))。
// 区間の最小は、覆う節ごとに、橋の 2 点の低い側の子へ下りる (O(log N))。
//
// 想定解からの変更は 3 つ。(1) 橋を探し直すとき、祖先の作用を足さない。点を同じ 1 次関数でずらしても外積の
// 符号は変わらない。(2) 外積を i64 で取る。値は 2 * 10^12 ほどまでで、外積は 10^18 ほどに収まる。交点の位置を
// 比べる所だけ i128 にする。(3) 区間の最小を根から下りる再帰にして、覆う節の祖先の作用を道で足しながら持つ。
struct Solver {
  struct Pt {
    i64 x, y;
  };
  struct Lin {
    i64 m, k;  // y += m x + k
  };
  struct Bridge {
    Pt p, q;  // 左の子の凸包の点と右の子の凸包の点。節の作用とその祖先の作用を除いた値
  };
  int sz = 1, lg = 0;
  vector<i64> y0;       // 葉の値 (作用を除いた値)
  vector<Lin> lz;       // 節に積んだ作用
  vector<Bridge> br;    // 内部の節の橋

  static i64 cross(Pt a, Pt b, Pt c) {  // (b - a) x (c - a)
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }
  static Pt mv(Pt p, Lin f) { return {p.x, p.y + f.m * p.x + f.k}; }

  explicit Solver(const vector<i64> &a) {
    const int n = int(a.size());
    sz = int(bit_ceil(unsigned(max(n, 1)))), lg = countr_zero(unsigned(sz));
    y0.assign(sz, 0);
    for (int i = 0; i < n; ++i) y0[i] = a[i];
    lz.assign(2 * sz, {0, 0});
    br.resize(sz);
    for (int v = sz - 1; v >= 1; --v) rebuild(v);
  }

  // 節 u の凸包の左端と右端を表す橋。葉なら 1 点。
  Pt leaf(int u) const { return {u - sz, y0[u - sz]}; }

  // 節 v の左右の子の凸包を結ぶ橋を探し直す。
  void rebuild(int v) {
    int a = 2 * v, b = 2 * v + 1;
    Lin fa = lz[a], fb = lz[b];
    i64 border = b;  // 右の子の最初の葉の位置
    while (border < sz) border <<= 1;
    border -= sz;
    while (a < sz || b < sz) {
      Pt p0, p1, q0, q1;
      if (a < sz) p0 = mv(br[a].p, fa), p1 = mv(br[a].q, fa);
      else p0 = p1 = mv(leaf(a), fa);
      if (b < sz) q0 = mv(br[b].p, fb), q1 = mv(br[b].q, fb);
      else q0 = q1 = mv(leaf(b), fb);
      if (a < sz && cross(p0, p1, q0) < 0) {
        a = 2 * a, fa = {fa.m + lz[a].m, fa.k + lz[a].k};
      } else if (b < sz && cross(p1, q0, q1) < 0) {
        b = 2 * b + 1, fb = {fb.m + lz[b].m, fb.k + lz[b].k};
      } else if (a >= sz) {
        b = 2 * b, fb = {fb.m + lz[b].m, fb.k + lz[b].k};
      } else if (b >= sz) {
        a = 2 * a + 1, fa = {fa.m + lz[a].m, fa.k + lz[a].k};
      } else {
        // 直線 p0 p1 と直線 q0 q1 の交点が、右の子の始まりより左にあるか。
        const __int128 c1 = __int128(p1.x - p0.x) * (q1.y - q0.y) - __int128(p1.y - p0.y) * (q1.x - q0.x);
        const __int128 c2 = __int128(p1.x - p0.x) * (p1.y - q0.y) - __int128(p1.y - p0.y) * (p1.x - q0.x);
        const bool left = (c1 == 0 && c2 == 0) ? q0.x < border : __int128(q0.x) * c1 + __int128(q1.x - q0.x) * c2 < c1 * border;
        if (left) a = 2 * a + 1, fa = {fa.m + lz[a].m, fa.k + lz[a].k};
        else b = 2 * b, fb = {fb.m + lz[b].m, fb.k + lz[b].k};
      }
    }
    br[v] = {mv(leaf(a), fa), mv(leaf(b), fb)};
  }

  // 節 v の全体の最小。f は v とその祖先の作用の和。
  i64 subtree_min(int v, Lin f) const {
    while (v < sz) {
      const Pt p = mv(br[v].p, f), q = mv(br[v].q, f);
      v = p.y < q.y ? 2 * v : 2 * v + 1;
      f = {f.m + lz[v].m, f.k + lz[v].k};
    }
    return mv(leaf(v), f).y;
  }

  i64 min_rec(int v, int lo, int hi, int l, int r, Lin f) const {
    f = {f.m + lz[v].m, f.k + lz[v].k};
    if (l <= lo && hi <= r) return subtree_min(v, f);
    const int mid = (lo + hi) >> 1;
    i64 res = numeric_limits<i64>::max();
    if (l < mid) res = min(res, min_rec(2 * v, lo, mid, l, r, f));
    if (r > mid) res = min(res, min_rec(2 * v + 1, mid, hi, l, r, f));
    return res;
  }

  void add(int l, int r, i64 b, i64 c) {
    l += sz, r += sz;
    for (int a = l, z = r; a < z; a >>= 1, z >>= 1) {
      if (a & 1) lz[a].m += b, lz[a].k += c, ++a;
      if (z & 1) --z, lz[z].m += b, lz[z].k += c;
    }
    for (int i = 1; i <= lg; ++i) {
      if (((l >> i) << i) != l) rebuild(l >> i);
      if (((r >> i) << i) != r) rebuild((r - 1) >> i);
    }
  }

  i64 minimum(int l, int r) const { return min_rec(1, 0, sz, l, r, {0, 0}); }
};
