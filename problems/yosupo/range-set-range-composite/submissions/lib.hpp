#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree に、1 次関数の合成のモノイドと代入の作用を載せる。長さ sz の区間に
// g を代入した値 g^sz は、作用を当てるたびに二分累乗で求める。
struct Solver {
  using Mint = ModInt<998244353>;
  using F = pair<Mint, Mint>;  // x -> first x + second
  static F compose(const F &l, const F &r) { return {r.first * l.first, r.first * l.second + r.second}; }
  struct RsetComposite {
    using T = F;
    using E = F;
    static T ti() { return {Mint(1), Mint(0)}; }
    static T op(const T &l, const T &r) { return compose(l, r); }
    static void mp(T &v, const E &f, int sz) {
      T res = ti(), b = f;
      for (; sz; sz >>= 1, b = compose(b, b))
        if (sz & 1) res = compose(res, b);
      v = res;
    }
    static void cp(E &pre, const E &suf) { pre = suf; }
  };
  SegmentTree<RsetComposite> seg;

  static vector<F> conv(const vector<array<i64, 2>> &f) {
    vector<F> v(f.size());
    for (size_t i = 0; i < f.size(); ++i) v[i] = {Mint(f[i][0]), Mint(f[i][1])};
    return v;
  }
  explicit Solver(const vector<array<i64, 2>> &f) : seg(conv(f)) {}
  void assign(int l, int r, i64 c, i64 d) { seg.apply(l, r, {Mint(c), Mint(d)}); }
  i64 query(int l, int r, i64 x) {
    const F t = seg.prod(l, r);
    return (t.first * Mint(x) + t.second).val();
  }
};
