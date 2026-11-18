#pragma once
// 案 C の試作 (_shared/matroid/flex.hpp)。マトロイドの型は can_exchange と circuit の両方を持つ。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/flex.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), edges(edges), color(color), cap(cap) {}

  void run() {
    mtr::Graphic g0(n, edges);
    mtr::Partition p0(color, cap);
    auto &g = g0;
    auto &p = p0;
    ans = flex::matroid_intersection(g, p);
  }

  vector<int> answer() const { return ans; }
};
