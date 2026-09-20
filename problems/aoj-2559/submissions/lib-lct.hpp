#pragma once
#include "common.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// 最小全域木の辺を頂点に置き換えて Link-Cut 木に載せる。木に入らなかった辺
// ごとに、その両端を結ぶ道へ chmin を流す。列に潰さないので、道の分解が要らない。
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

    // 辺に値を持たせたいので、辺ごとに頂点を 1 つ足して 2 本の辺で繋ぐ。
    LinkCutTree<Chmin> lct(2 * n - 1, NONE);
    vector<int> id(m, -1);
    int num = n;
    for (int i = 0; i < m; ++i)
      if (mst.used[i]) {
        id[i] = num++;
        lct.link((int)edges[i][0], id[i]), lct.link(id[i], (int)edges[i][1]);
      }
    for (int i = 0; i < m; ++i)
      if (id[i] == -1)
        lct.apply((int)edges[i][0], (int)edges[i][1], (int)edges[i][2]);

    ans.resize(m);
    for (int i = 0; i < m; ++i) {
      if (id[i] == -1) {
        ans[i] = mst.cost;
        continue;
      }
      i64 alt = lct[id[i]];
      ans[i] = alt == NONE ? -1 : mst.cost - edges[i][2] + alt;
    }
  }

  const vector<i64> &answer() const { return ans; }
};
