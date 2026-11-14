#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/SegmentTree.hpp"

// NeoLibrary の SegmentTree に、1 次関数の合成と代入の作用を自前の構造体で渡す (中身は二分木の汎用の遅延セグ木)。長さ len の
// 区間の g^len は、作用を当てるたびに二分累乗で求める (今の Library の lib と同じ書き方)。標準の構造体 monoid::AssignProd を
// 渡す lib-neo と比べるために置く。
struct Solver {
  using Z = ZMod<998244353>;
  using F = array<Z, 2>;
  static F compose(const F &l, const F &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  struct M {
    using T = F;
    using E = F;
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return compose(l, r); }
    static void mp(T &v, const E &f, int len) {
      T res = ti(), b = f;
      for (; len; len >>= 1, b = compose(b, b))
        if (len & 1) res = compose(res, b);
      v = res;
    }
    static void cp(E &pre, const E &suf) { pre = suf; }
  };
  SegmentTree<M> seg;
  explicit Solver(const vector<array<i64, 2>> &f) : seg(int(f.size()), [&](int i) { return F{Z(f[i][0]), Z(f[i][1])}; }) {}
  void assign(int l, int r, i64 c, i64 d) { seg.apply(l, r, {Z(c), Z(d)}); }
  i64 query(int l, int r, i64 x) {
    const F t = seg.prod(l, r);
    return (t[0] * Z(x) + t[1]).val();
  }
};
