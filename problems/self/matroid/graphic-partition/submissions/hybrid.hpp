#pragma once
// 層は閉路の列挙で作り、増加路は 2 引数の判定でまとめて探す方法 (intersect_hybrid)。
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
    ans = mi_algo::intersect_hybrid((int)edges.size(), g, p);
  }

  vector<int> answer() const { return ans; }
};
