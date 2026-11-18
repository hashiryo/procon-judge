#pragma once
#include "common.hpp"
#include "neo/data_structure/SegmentTree.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// NeoLibrary の HeavyLightDecomposition と SegmentTree (作用だけの双対の形) で、lib-hld-segtree と同じ形に書いたもの。最小全域木を
// 作るところは common.hpp の共通の関数 (今の Library の UnionFind)。木に入らなかった辺ごとに、その両端を結ぶ道の辺 (子の側の
// 頂点で代表する) へ重みの chmin を流す。
struct Solver {
  struct Chmin {
    using T = int;
    using E = int;
    static void mp(T &v, const E &f) {
      if (v > f) v = f;
    }
    static void cp(E &pre, const E &suf) {
      if (pre > suf) pre = suf;
    }
  };
  int n;
  vector<array<i64, 3>> edges;
  vector<i64> ans;
  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}
  void run() {
    const int m = (int)edges.size();
    Mst mst = build_mst(n, edges);
    vector<array<int, 2>> te;
    for (int i = 0; i < m; ++i)
      if (mst.used[i]) te.push_back({(int)edges[i][0], (int)edges[i][1]});
    HeavyLightDecomposition h(n, te);
    SegmentTree<Chmin> seg(n, NONE);
    vector<int> child(m);
    for (int i = m; i--;) {
      int u = (int)edges[i][0], v = (int)edges[i][1];
      if (mst.used[i]) child[i] = h.in_subtree(u, v) ? u : v;  // 辺は深い側の頂点で代表する
      else
        h.path(u, v, [&](int x, int y) { x <= y ? seg.apply(x, y + 1, (int)edges[i][2]) : seg.apply(y, x + 1, (int)edges[i][2]); }, true);
    }
    ans.resize(m);
    for (int i = 0; i < m; ++i) {
      if (!mst.used[i]) {
        ans[i] = mst.cost;
        continue;
      }
      const int alt = seg[h.to_seq(child[i])];
      ans[i] = alt == NONE ? -1 : mst.cost - edges[i][2] + alt;
    }
  }
  const vector<i64> &answer() const { return ans; }
};
