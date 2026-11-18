#pragma once
// ポテンシャル付きの Dijkstra で、辺は 2 引数の交換の判定 (can_exchange) で探す (wmi_dijkstra<false>)。
// M1 をグラフ的マトロイド、M2 を分割マトロイドにする。マトロイドの型と算法は _shared/matroid/ にある。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/wmi_algos.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  wmi_algo::Result res;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), edges(edges), color(color), cap(cap), w(w) {}

  void run() {
    mtr::Graphic g(n, edges);
    mtr::Partition p(color, cap);
    res = wmi_algo::wmi_dijkstra<false>((int)edges.size(), g, p, w);
  }

  vector<i64> answer() const { return res.best; }
  vector<int> chosen() const { return res.set; }
};
