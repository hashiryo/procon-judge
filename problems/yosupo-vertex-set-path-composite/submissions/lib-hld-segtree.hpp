#pragma once
#include <algorithm>
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// HLD で木を列に潰すと道が O(log N) 本の区間になるが、合成は可換でないので、
// 列を反転したセグメント木をもう 1 本持って、登る向きで使い分ける。
struct Solver {
  int n;
  HeavyLightDecomposition hld;
  SegmentTree<Composite> seg, rseg;

  static HeavyLightDecomposition make_hld(int n,
                                          const vector<array<int, 2>> &edges) {
    Graph g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    return HeavyLightDecomposition(g, 0);
  }

  static vector<Composite::T> to_seq_order(const HeavyLightDecomposition &h,
                                           const vector<array<i64, 2>> &f) {
    vector<Composite::T> v(f.size());
    for (int u = (int)f.size(); u--;) v[h.to_seq(u)] = {Mint(f[u][0]), Mint(f[u][1])};
    return v;
  }

  static vector<Composite::T> reversed(vector<Composite::T> v) {
    return std::reverse(v.begin(), v.end()), v;
  }

  Solver(int n, const vector<array<i64, 2>> &f,
         const vector<array<int, 2>> &edges)
      : n(n), hld(make_hld(n, edges)), seg(to_seq_order(hld, f)),
        rseg(reversed(to_seq_order(hld, f))) {}

  void set(int p, i64 c, i64 d) {
    int i = hld.to_seq(p);
    seg.set(i, {Mint(c), Mint(d)});
    rseg.set(n - i - 1, {Mint(c), Mint(d)});
  }

  i64 composite(int u, int v, i64 x) {
    auto acc = Composite::ti();
    for (auto [s, t] : hld.path(u, v))
      acc = Composite::op(acc, s < t ? seg.prod(s, t + 1)
                                     : rseg.prod(n - s - 1, n - t));
    return (acc[0] * Mint(x) + acc[1]).val();
  }
};
