#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// HLD で木を列に潰す。和の最大は左右の向きを区別するので、列を反転した
// セグメント木をもう 1 本持って、登る向きで使い分ける。
struct Solver {
  struct MaxSum {
    static inline i64 INF = 1LL << 60;
    struct T {
      i64 sum, max, lmax, rmax;
    };
    using E = i64;
    static T ti() { return {0, -INF, -INF, -INF}; }
    static T op(const T &a, const T &b) {
      return {a.sum + b.sum, std::max({a.max, b.max, a.rmax + b.lmax}),
              std::max(a.lmax, a.sum + b.lmax), std::max(a.rmax + b.sum, b.rmax)};
    }
    // 区間を全部 f にするので、長さが分かれば和がそのまま決まる。
    static void mp(T &v, const E &f, int sz) {
      v.sum = f * sz;
      v.max = v.lmax = v.rmax = std::max(v.sum, f);
    }
    static void cp(E &pre, const E &suf) { pre = suf; }
  };

  int n;
  HeavyLightDecomposition hld;
  SegmentTree<MaxSum> seg, rseg;

  static HeavyLightDecomposition make_hld(int n,
                                          const vector<array<int, 2>> &edges) {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<typename MaxSum::T> to_seq_order(const HeavyLightDecomposition &h,
                                                 const vector<i64> &w) {
    vector<typename MaxSum::T> v(w.size());
    for (int u = (int)w.size(); u--;) v[h.to_seq(u)] = {w[u], w[u], w[u], w[u]};
    return v;
  }

  static vector<typename MaxSum::T> reversed(vector<typename MaxSum::T> v) {
    return std::reverse(v.begin(), v.end()), v;
  }

  Solver(int n, const vector<i64> &w, const vector<array<int, 2>> &edges)
      : n(n), hld(make_hld(n, edges)), seg(to_seq_order(hld, w)),
        rseg(reversed(to_seq_order(hld, w))) {}

  void assign(int u, int v, i64 c) {
    for (auto [x, y] : hld.path(u, v))
      if (x < y) seg.apply(x, y + 1, c), rseg.apply(n - y - 1, n - x, c);
      else seg.apply(y, x + 1, c), rseg.apply(n - x - 1, n - y, c);
  }

  i64 max_sum(int u, int v) {
    auto acc = MaxSum::ti();
    for (auto [x, y] : hld.path(u, v))
      acc = MaxSum::op(acc, x < y ? seg.prod(x, y + 1) : rseg.prod(n - x - 1, n - y));
    return acc.max;
  }
};
