#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library の SegmentTree。raw だった頃の lib.cpp と同じモノイド (pair<Mint, Mint>) を載せる。
struct Solver {
  using Mint = ModInt<998244353>;
  struct RcompositeQ {
    using T = pair<Mint, Mint>;
    static T ti() { return make_pair(Mint(1), Mint(0)); }
    static T op(const T &l, const T &r) { return make_pair(r.first * l.first, r.first * l.second + r.second); }
  };
  SegmentTree<RcompositeQ> seg;

  static vector<RcompositeQ::T> conv(const vector<array<i64, 2>> &f) {
    vector<RcompositeQ::T> v(f.size());
    for (size_t i = 0; i < f.size(); ++i) v[i] = {Mint(f[i][0]), Mint(f[i][1])};
    return v;
  }

  explicit Solver(const vector<array<i64, 2>> &f) : seg(conv(f)) {}

  void set(int p, i64 c, i64 d) { seg.set(p, {Mint(c), Mint(d)}); }

  i64 query(int l, int r, i64 x) {
    const auto t = seg.prod(l, r);
    return (t.first * Mint(x) + t.second).val();
  }
};
