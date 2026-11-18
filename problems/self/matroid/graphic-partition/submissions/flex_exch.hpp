#pragma once
// 案 C の試作 (_shared/matroid/flex.hpp)。マトロイドの型に can_exchange だけを見せ、circuit は Adapter が I の要素を全部調べて補う。
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
    flex::OnlyExchange<decltype(g0)> g{g0};
    flex::OnlyExchange<decltype(p0)> p{p0};
    ans = flex::matroid_intersection(g, p);
  }

  vector<int> answer() const { return ans; }
};
