#pragma once
// ライブラリを使う提出が共有するもの。二重辺連結成分への縮約と、成分ごとの
// 獲物の持ち方はどちらの実装でも同じなので、ここに置く。比べたいのは木の上の
// 道の最大を取る構造。
#include <queue>
#include <utility>
#include "pj.hpp"
#include "mylib/graph/IncrementalBridgeConnectivity.hpp"

// 二重辺連結成分に縮約した木。
struct BridgeTree {
  int n = 0;                    // 成分の数
  vector<int> id;               // 街 -> 成分
  vector<array<int, 2>> edges;  // 橋 (成分どうしを結ぶ辺)
};

inline BridgeTree contract(int n, const vector<array<int, 2>> &edges) {
  IncrementalBridgeConnectivity ibc(n);
  for (auto &e : edges) ibc.add_edge(e[0], e[1]);
  BridgeTree t;
  vector<int> of_leader(n, -1);
  t.id.resize(n);
  for (int v = 0; v < n; ++v) {
    int l = ibc.leader(v);
    if (of_leader[l] < 0) of_leader[l] = t.n++;
    t.id[v] = of_leader[l];
  }
  for (auto &e : edges) {
    int u = t.id[e[0]], v = t.id[e[1]];
    if (u != v) t.edges.push_back({u, v});
  }
  return t;
}

// 成分ごとの獲物。無いときの値は -1。
struct Prey {
  vector<priority_queue<i64>> pq;

  explicit Prey(int n) : pq(n) {}

  void add(int c, i64 w) { pq[c].push(w); }
  i64 top(int c) const { return pq[c].empty() ? -1 : pq[c].top(); }
  void pop(int c) { pq[c].pop(); }
};

// (価値, 成分) の最大。無いところは {-1, -1}。
struct MaxPrey {
  using T = pair<i64, int>;
  static T ti() { return {-1, -1}; }
  static T op(const T &l, const T &r) { return l.first > r.first ? l : r; }
  using commute = void;
};
