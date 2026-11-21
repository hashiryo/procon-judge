#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/SegmentTree.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition と遅延の SegmentTree で、lib-hld-segtree と同じ形に書いたもの。作用素は common.hpp の
// Inflation と同じで、剰余を NeoLibrary の ZMod にした。和は可換なので、区間の向きは見ない。
struct Solver {
  using Z = ZMod<1000000007>;
  // {宿泊費の和, インフレ係数の和}。規模 z のパレードは s += c * z。作用の合成は z の和。
  struct Inflation {
    struct T {
      Z s, c;
    };
    using E = Z;
    static T ti() { return {Z(0), Z(0)}; }
    static T op(const T &l, const T &r) { return {l.s + r.s, l.c + r.c}; }
    static void mp(T &v, const E &z) { v.s += v.c * z; }
    static void cp(E &p, const E &z) { p += z; }
  };
  HeavyLightDecomposition h;
  SegmentTree<Inflation> seg;
  static vector<Inflation::T> by_pos(const HeavyLightDecomposition &h, const vector<i64> &s, const vector<i64> &c) {
    vector<Inflation::T> v(s.size());
    for (int u = 0; u < (int)s.size(); ++u) v[h.to_seq(u)] = {Z(s[u]), Z(c[u])};
    return v;
  }
  Solver(int n, const vector<i64> &s, const vector<i64> &c, const vector<array<int, 2>> &edges)
      : h(n, edges), seg(by_pos(h, s, c)) {}
  void parade(int x, int y, i64 z) {
    const Z w(z);
    h.path(x, y, [&](int a, int b) { a <= b ? seg.apply(a, b + 1, w) : seg.apply(b, a + 1, w); });
  }
  i64 travel(int x, int y) {
    Z acc(0);
    h.path(x, y, [&](int a, int b) { acc += (a <= b ? seg.prod(a, b + 1) : seg.prod(b, a + 1)).s; });
    return acc.val();
  }
};
