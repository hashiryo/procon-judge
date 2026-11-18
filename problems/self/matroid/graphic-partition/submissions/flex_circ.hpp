#pragma once
// 案 C の試作 (_shared/matroid/flex.hpp)。マトロイドの型に circuit だけを見せ、can_exchange は Adapter が閉路を覚えて補う。
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
    flex::OnlyCircuit<decltype(g0)> g{g0};
    flex::OnlyCircuit<decltype(p0)> p{p0};
    ans = flex::matroid_intersection(g, p);
  }

  vector<int> answer() const { return ans; }
};
