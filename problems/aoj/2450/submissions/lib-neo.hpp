#pragma once
#include <algorithm>
#include "pj.hpp"
#include "neo/base/ds/range/SegmentTree.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition と SegmentTree で、lib-hld-segtree と同じ形に書いたもの。和の最大は左右の向きを区別
// するので、位置を逆に並べたセグメント木をもう 1 本持ち、上る区間 (l > r) は逆の木で取る。
struct Solver {
  struct MaxSum {
    static inline i64 INF = 1LL << 60;
    struct T {
      i64 sum, max, lmax, rmax;
    };
    using E = i64;
    static T ti() { return {0, -INF, -INF, -INF}; }
    static T op(const T &a, const T &b) {
      return {a.sum + b.sum, std::max({a.max, b.max, a.rmax + b.lmax}), std::max(a.lmax, a.sum + b.lmax),
              std::max(a.rmax + b.sum, b.rmax)};
    }
    // 区間を全部 f にするので、長さが分かれば和がそのまま決まる。
    static void mp(T &v, const E &f, int sz) {
      v.sum = f * sz;
      v.max = v.lmax = v.rmax = std::max(v.sum, f);
    }
    static void cp(E &pre, const E &suf) { pre = suf; }
  };
  int n;
  HeavyLightDecomposition h;
  SegmentTree<MaxSum> seg, rseg;
  static vector<MaxSum::T> by_pos(const HeavyLightDecomposition &h, const vector<i64> &w, bool rev) {
    const int n = (int)w.size();
    vector<MaxSum::T> v(n);
    for (int u = 0; u < n; ++u) v[rev ? n - 1 - h.to_seq(u) : h.to_seq(u)] = {w[u], w[u], w[u], w[u]};
    return v;
  }
  Solver(int n_, const vector<i64> &w, const vector<array<int, 2>> &edges)
      : n(n_), h(n_, edges), seg(by_pos(h, w, false)), rseg(by_pos(h, w, true)) {}
  void assign(int u, int v, i64 c) {
    h.path(u, v, [&](int x, int y) {
      if (x > y) std::swap(x, y);
      seg.apply(x, y + 1, c), rseg.apply(n - 1 - y, n - x, c);
    });
  }
  i64 max_sum(int u, int v) {
    auto acc = MaxSum::ti();
    h.path(u, v, [&](int x, int y) { acc = MaxSum::op(acc, x <= y ? seg.prod(x, y + 1) : rseg.prod(n - 1 - x, n - y)); });
    return acc.max;
  }
};
