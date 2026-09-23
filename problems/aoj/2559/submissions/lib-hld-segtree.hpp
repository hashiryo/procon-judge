#pragma once
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 最小全域木を HLD で列に潰して、木に入らなかった辺ごとに、その両端を結ぶ道へ
// 重みの chmin を流す。作用だけなので値の側のモノイドは要らない。
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

    Graph tree(n);
    for (int i = 0; i < m; ++i)
      if (mst.used[i]) tree.add_edge((int)edges[i][0], (int)edges[i][1]);
    HeavyLightDecomposition hld(tree, 0);

    SegmentTree<Chmin> seg(n, NONE);
    // 木に入らなかった辺を、その道の上の辺すべての代わりの候補として流す。
    vector<array<int, 2>> ends(m);
    for (int i = m; i--;) {
      int u = (int)edges[i][0], v = (int)edges[i][1];
      if (mst.used[i]) {
        // 辺は深い側の頂点で代表する。
        if (hld.in_subtree(u, v)) std::swap(u, v);
        ends[i] = {u, v};
      } else
        for (auto [x, y] : hld.path(u, v, true))
          x < y ? seg.apply(x, y + 1, (int)edges[i][2])
                : seg.apply(y, x + 1, (int)edges[i][2]);
    }

    ans.resize(m);
    for (int i = 0; i < m; ++i) {
      if (!mst.used[i]) {
        ans[i] = mst.cost;
        continue;
      }
      int alt = seg[hld.to_seq(ends[i][1])];
      ans[i] = alt == NONE ? -1 : mst.cost - edges[i][2] + alt;
    }
  }

  const vector<i64> &answer() const { return ans; }
};
