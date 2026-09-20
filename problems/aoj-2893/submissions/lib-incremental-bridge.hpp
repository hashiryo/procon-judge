#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/IncrementalBridgeConnectivity.hpp"

// 二重辺連結成分で縮約して橋の木を作り、1 回の走査で部分木の重みを出す。
// 辺を 1 本抜いたときの分かれ方は木の上で決まるので、全部の辺を O(1) で
// 見られる。
struct Solver {
  int n;
  vector<array<i64, 3>> edges;
  array<i64, 2> ans{0, 0};

  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  void run() {
    const int m = (int)edges.size();
    IncrementalBridgeConnectivity ibc(n);
    for (auto &e : edges) ibc.add_edge((int)e[0], (int)e[1]);

    vector<int> id(n);
    int k = 0;
    for (int i = 0; i < n; ++i)
      if (ibc.leader(i) == i) id[i] = k++;

    // 縮約した成分を頂点にした木を作る。成分の中に閉じた辺の重みは頂点へ足す。
    Graph t(k);
    vector<i64> s(k, 0), tw;
    for (int e = m; e--;) {
      int u = id[ibc.leader((int)edges[e][0])], v = id[ibc.leader((int)edges[e][1])];
      if (u == v) s[u] += edges[e][2];
      else t.add_edge(u, v), tw.push_back(edges[e][2]);
    }

    auto adj = t.adjacency_edge(0);
    auto dfs = [&](auto &&self, int v, int p) -> void {
      for (int e : adj[v])
        if (int u = t[e].to(v); u != p) self(self, u, v), s[v] += s[u] + tw[e];
    };
    dfs(dfs, 0, -1);

    i64 best = 1LL << 60;
    for (int e = 0; e < m; ++e) {
      int u = (int)edges[e][0], v = (int)edges[e][1];
      int cu = id[ibc.leader(u)], cv = id[ibc.leader(v)];
      i64 cost;
      if (cu == cv) cost = s[0] - edges[e][2];  // 橋でないので分かれない
      else {
        if (s[cu] > s[cv]) std::swap(cu, cv);
        cost = std::abs(s[cu] - (s[0] - s[cu] - edges[e][2]));
      }
      if (best > cost) best = cost, ans = {u, v};
      else if (best == cost) {
        if (ans[0] > u) ans = {u, v};
        else if (ans[0] == u && ans[1] > v) ans[1] = v;
      }
    }
  }

  array<i64, 2> answer() const { return ans; }
};
