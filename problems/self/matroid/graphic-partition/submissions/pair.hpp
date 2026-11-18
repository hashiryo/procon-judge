#pragma once
// 2 引数の交換の判定 (can_exchange) だけを使う Cunningham の方法 (intersect_pair)。
// M1 をグラフ的マトロイド、M2 を分割マトロイドにする。マトロイドの型と算法は _shared/matroid/ にある。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/mi_algos.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), edges(edges), color(color), cap(cap) {}

  void run() {
    mtr::Graphic g(n, edges);
    mtr::Partition p(color, cap);
    ans = mi_algo::intersect_pair((int)edges.size(), g, p);
  }

  vector<int> answer() const { return ans; }
};
