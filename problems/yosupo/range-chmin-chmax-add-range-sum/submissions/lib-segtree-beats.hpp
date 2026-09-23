#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// Segment Tree Beats。作用が全体に効くかを mp が真偽で返し、効かないノードは
// 子へ降りて作り直す。最大値と 2 番目の最大値 (最小側も同様) を持って、
// 降りる回数がならすと O((N+Q) log^2 N) に収まることを使う。
struct Solver {
  struct Beats {
    static constexpr i64 INF = 1LL << 62;
    struct T {
      i64 sum, h, l, h2, l2;
      int hc, lc;
    };
    // clamp(x, lb, ub) + ad
    struct E {
      i64 lb, ub, ad;
      static E add(i64 x) { return {-INF, INF, x}; }
      static E chmin(i64 x) { return {-INF, x, 0}; }
      static E chmax(i64 x) { return {x, INF, 0}; }
    };
    static i64 min2(i64 a, i64 a2, i64 b, i64 b2) {
      return a == b ? min(a2, b2) : a2 <= b ? a2 : b2 <= a ? b2 : max(a, b);
    }
    static i64 max2(i64 a, i64 a2, i64 b, i64 b2) {
      return a == b ? max(a2, b2) : a2 >= b ? a2 : b2 >= a ? b2 : min(a, b);
    }
    static T ti() { return {0, -INF, INF, -INF, INF, 0, 0}; }
    static T op(const T &vl, const T &vr) {
      return {vl.sum + vr.sum,
              max(vl.h, vr.h),
              min(vl.l, vr.l),
              max2(vl.h, vl.h2, vr.h, vr.h2),
              min2(vl.l, vl.l2, vr.l, vr.l2),
              vl.hc * (vl.h >= vr.h) + vr.hc * (vl.h <= vr.h),
              vl.lc * (vl.l <= vr.l) + vr.lc * (vl.l >= vr.l)};
    }
    static bool mp(T &v, const E &f, int sz) {
      if (v.h <= f.lb) {
        v.sum = (v.h = v.l = f.lb + f.ad) * (v.hc = v.lc = sz);
        v.h2 = -INF, v.l2 = INF;
        return true;
      }
      if (v.l >= f.ub) {
        v.sum = (v.h = v.l = f.ub + f.ad) * (v.hc = v.lc = sz);
        v.h2 = -INF, v.l2 = INF;
        return true;
      }
      if (f.lb <= v.l && v.h <= f.ub) {
        v.sum += f.ad * sz, v.h += f.ad, v.l += f.ad;
        v.h2 += f.ad, v.l2 += f.ad;
        return true;
      }
      if (v.h2 <= f.lb) {
        v.l = v.h2 = f.lb + f.ad, v.lc = sz - v.hc;
        v.l2 = v.h = min(f.ub, v.h) + f.ad;
        v.sum = v.h * v.hc + v.l * v.lc;
        return true;
      }
      if (v.l2 >= f.ub) {
        v.h = v.l2 = f.ub + f.ad, v.hc = sz - v.lc;
        v.h2 = v.l = max(f.lb, v.l) + f.ad;
        v.sum = v.h * v.hc + v.l * v.lc;
        return true;
      }
      return false;
    }
    static void cp(E &pre, const E &suf) {
      if (auto tl = suf.lb - pre.ad; pre.ub <= tl) pre.ub = pre.lb = tl;
      else if (auto tu = suf.ub - pre.ad; tu <= pre.lb) pre.ub = pre.lb = tu;
      else pre.lb = max(pre.lb, tl), pre.ub = min(pre.ub, tu);
      pre.ad += suf.ad;
    }
  };

  SegmentTree<Beats> seg;

  explicit Solver(const vector<i64> &a) : seg((int)a.size()) {
    for (int i = 0; i < (int)a.size(); ++i)
      seg.unsafe_set(i, {a[i], a[i], a[i], -Beats::INF, Beats::INF, 1, 1});
    seg.build();
  }

  void chmin(int l, int r, i64 b) { seg.apply(l, r, Beats::E::chmin(b)); }

  void chmax(int l, int r, i64 b) { seg.apply(l, r, Beats::E::chmax(b)); }

  void add(int l, int r, i64 b) { seg.apply(l, r, Beats::E::add(b)); }

  i64 sum(int l, int r) { return seg.prod(l, r).sum; }
};
