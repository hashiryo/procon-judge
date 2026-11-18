#pragma once
// 今の Library の matroid_intersection (交換の判定を 2 引数で呼び、最短の増加路を層ごとにまとめて探す Cunningham の方法)
// と、GraphicMatroid、PartitionMatroid をそのまま使う。
#include "mylib/optimization/matroid_intersection.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), edges(edges), color(color), cap(cap) {}

  void run() {
    const int m = edges.size();
    GraphicMatroid g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    vector<vector<int>> parts(cap.size());
    for (int i = 0; i < m; ++i) parts[color[i]].push_back(i);
    PartitionMatroid p(m, parts, cap);
    ans = matroid_intersection(m, g, p);
  }

  vector<int> answer() const { return ans; }
};
