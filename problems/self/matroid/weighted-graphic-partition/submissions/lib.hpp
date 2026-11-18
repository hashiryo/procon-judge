#pragma once
// 今の Library の weighted_matroid_intersection<MAXIMIZE> (重みを n + 1 倍して要素ごとに 1 を引き、優先度つきキューで
// 交換グラフの最長路を求める。辺は 2 引数の判定 oracle(e, f) で調べる) と、GraphicMatroid、PartitionMatroid を使う。
// 返る大きさ k ごとの集合から重みの和を数える。
#include "mylib/optimization/matroid_intersection.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  vector<i64> best;
  vector<int> I;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), edges(edges), color(color), cap(cap), w(w) {}

  void run() {
    const int m = edges.size();
    GraphicMatroid g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    vector<vector<int>> parts(cap.size());
    for (int i = 0; i < m; ++i) parts[color[i]].push_back(i);
    PartitionMatroid p(m, parts, cap);
    auto S = weighted_matroid_intersection<MAXIMIZE>(m, g, p, w);
    best.clear();
    for (auto &s : S) {
      i64 sum = 0;
      for (int x : s) sum += w[x];
      best.push_back(sum);
    }
    I = S.back();
  }

  vector<i64> answer() const { return best; }
  vector<int> chosen() const { return I; }
};
