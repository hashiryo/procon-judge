#pragma once
#include <queue>
#include <utility>
#include "pj.hpp"
#include "neo/base/ds/range/SegmentTree.hpp"
#include "neo/base/ds/tree/HeavyLightDecomposition.hpp"
#include "neo/algo/decide/graph/two_edge_connected_components.hpp"

// NeoLibrary で lib-hld-segtree と同じ形に書いたもの。二重辺連結成分への縮約は two_edge_connected_components、橋だけの木の上の
// 道の最大は HeavyLightDecomposition と SegmentTree。成分ごとの獲物の持ち方は common.hpp と同じ (優先度付きキュー)。
struct Solver {
  // (価値, 成分) の最大。無いところは {-1, -1}。
  struct MaxPrey {
    using T = pair<i64, int>;
    static T ti() { return {-1, -1}; }
    static T op(const T &l, const T &r) { return l.first > r.first ? l : r; }
  };
  vector<int> id;  // 街 -> 成分
  vector<priority_queue<i64>> pq;
  HeavyLightDecomposition h;
  SegmentTree<MaxPrey> seg;
  static HeavyLightDecomposition bridge_tree(int n, const vector<array<int, 2>> &edges, vector<int> &id) {
    auto cc = two_edge_connected_components(n, edges);
    id = cc.comp;
    vector<array<int, 2>> te;
    for (auto &e : edges)
      if (id[e[0]] != id[e[1]]) te.push_back({id[e[0]], id[e[1]]});
    return HeavyLightDecomposition(cc.count, te);
  }
  static vector<MaxPrey::T> initial(const HeavyLightDecomposition &h) {
    const int n = (int)h.size();
    vector<MaxPrey::T> v(n);
    for (int c = 0; c < n; ++c) v[h.to_seq(c)] = {-1, c};
    return v;
  }
  Solver(int n, const vector<array<int, 2>> &edges) : h(bridge_tree(n, edges, id)), seg(initial(h)) { pq.resize(h.size()); }
  i64 top(int c) const { return pq[c].empty() ? -1 : pq[c].top(); }
  void add(int u, i64 w) {
    const int c = id[u];
    pq[c].push(w);
    seg.set(h.to_seq(c), {top(c), c});
  }
  i64 take(int s, int t) {
    const int u = id[s], v = id[t];
    MaxPrey::T best = MaxPrey::ti();
    h.path(u, v, [&](int x, int y) { best = MaxPrey::op(best, x <= y ? seg.prod(x, y + 1) : seg.prod(y, x + 1)); });
    if (best.first == -1) return -1;
    const int c = best.second;
    pq[c].pop();
    seg.set(h.to_seq(c), {top(c), c});
    return best.first;
  }
};
