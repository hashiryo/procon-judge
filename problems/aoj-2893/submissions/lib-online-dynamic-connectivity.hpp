#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/data_structure/OnlineDynamicConnectivity.hpp"

// 動的連結性の構造に全部の辺を入れておいて、1 本ずつ外しては戻す。縮約も
// 木への変換も要らない代わりに、1 回の cut と link が O(log^2 N) かかる。
struct Solver {
  struct Sum {
    using T = i64;
    static T ti() { return 0; }
    static T op(T l, T r) { return l + r; }
  };

  int n;
  vector<array<i64, 3>> edges;
  array<i64, 2> ans{0, 0};

  Solver(int n, const vector<array<i64, 3>> &edges) : n(n), edges(edges) {}

  void run() {
    OnlineDynamicConnectivity<Sum> dc(n);
    // 辺の重みを両端の頂点に半分ずつ持たせる。成分の総和を 2 で割れば
    // その成分の辺の重みになる。
    for (auto &e : edges) {
      int u = (int)e[0], v = (int)e[1];
      dc.link(u, v);
      dc.set(u, dc[u] + e[2]), dc.set(v, dc[v] + e[2]);
    }
    i64 best = 1LL << 60;
    for (auto &e : edges) {
      int u = (int)e[0], v = (int)e[1];
      dc.cut(u, v);
      i64 cost;
      if (dc.connected(u, v)) cost = dc.prod(u) / 2 - e[2];
      else cost = std::abs(dc.prod(u) - dc.prod(v)) / 2;
      if (best > cost) best = cost, ans = {u, v};
      else if (best == cost) {
        if (ans[0] > u) ans = {u, v};
        else if (ans[0] == u && ans[1] > v) ans[1] = v;
      }
      dc.link(u, v);
    }
  }

  array<i64, 2> answer() const { return ans; }
};
