#pragma once
// ライブラリを使う提出が共有するもの。木を HLD で列に潰し、灯籠の追加を「流した
// 街の位置 x、時刻 + 根からの距離 を y とする点への +1」と「消えたあとに通る最初
// の街の位置での -1」に、回答を「部分木の区間 x [0, 時刻 + 根からの距離] の総和」
// に直すところはどちらの実装でも同じなので、ここに置く。比べたいのは 2 次元の点
// 集合に対する 1 点更新と長方形の総和を取る構造。
#include <set>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/HeavyLightDecomposition.hpp"

struct RangeCount {
  using T = int;
  static T ti() { return 0; }
  static T op(T l, T r) { return l + r; }
};

// delta != 0 なら点 (x, y) への加算。delta == 0 なら [x0, x1) x [0, y] の総和の問い合わせ。
struct Event {
  int delta;
  i64 x0, x1, y;
};

struct Offline {
  set<array<i64, 2>> points;
  vector<Event> events;
};

inline Offline prepare(int n, const vector<array<i64, 3>> &edges,
                       const vector<array<i64, 4>> &qs) {
  Graph g(n);
  vector<i64> c(n - 1);
  for (int i = 0; i < n - 1; ++i) g.add_edge((int)edges[i][0], (int)edges[i][1]), c[i] = edges[i][2];
  HeavyLightDecomposition tree(g, 0);
  auto adj = g.adjacency_edge(0);
  vector<i64> dep(n);
  for (int i = 0; i < n; ++i) {
    int v = tree.to_vertex(i);
    for (int e : adj[v])
      if (int u = g[e].to(v); u != tree.parent(v)) dep[u] = dep[v] + c[e];
  }

  Offline o;
  for (auto &q : qs) {
    const int v = (int)q[1];
    const i64 t = q[2], l = q[3];
    if (q[0] == 0) {
      i64 x = tree.to_seq(v), y = t + dep[v];
      o.events.push_back({1, 0, x, y});
      o.points.insert({x, y});
      // 根へ向かって、灯籠が消えたあとに通る最初の街 u を探す。あれば、u より上の
      // 街では数えないように同じ y で -1 を置く。
      auto path = tree.path(0, v);
      int u = -1;
      for (int i = (int)path.size(); i--;) {
        auto [a, b] = path[i];
        if (dep[v] - dep[tree.to_vertex(a)] <= l) continue;
        for (++b; b - a > 1;) {
          int m = (a + b) / 2;
          (dep[v] - dep[tree.to_vertex(m)] > l ? a : b) = m;
        }
        u = tree.to_vertex(a);
        break;
      }
      if (u != -1) {
        x = tree.to_seq(u);
        o.events.push_back({-1, 0, x, y});
        o.points.insert({x, y});
      }
    } else {
      auto [lo, hi] = tree.subtree(v);
      o.events.push_back({0, lo, hi, t + dep[v]});
    }
  }
  return o;
}

// Engine は次を実装する。
//   Engine(const set<array<i64, 2>> &points);  // 点の集合。値は 0 で始まる
//   void add(i64 x, i64 y, int delta);          // 点の値に加える
//   int count(i64 x0, i64 x1, i64 ymax);        // [x0, x1) x [0, ymax] の総和
template <class Engine> struct LanternSolver {
  int n;
  vector<array<i64, 3>> edges;
  vector<array<i64, 4>> qs;
  vector<i64> ans;

  LanternSolver(int n, const vector<array<i64, 3>> &edges, const vector<array<i64, 4>> &qs)
      : n(n), edges(edges), qs(qs) {}

  void run() {
    Offline o = prepare(n, edges, qs);
    Engine eng(o.points);
    ans.clear();
    for (auto &e : o.events) {
      if (e.delta == 0) ans.push_back(eng.count(e.x0, e.x1, e.y));
      else eng.add(e.x1, e.y, e.delta);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
