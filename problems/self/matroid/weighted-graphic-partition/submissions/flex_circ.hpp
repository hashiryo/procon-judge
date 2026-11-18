#pragma once
// 案 C の試作 (_shared/matroid/flex.hpp)。マトロイドの型に circuit だけを見せ、can_exchange は Adapter が閉路を覚えて補う。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/flex.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  flex::Weighted res;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), edges(edges), color(color), cap(cap), w(w) {}

  void run() {
    mtr::Graphic g0(n, edges);
    mtr::Partition p0(color, cap);
    flex::OnlyCircuit<decltype(g0)> g{g0};
    flex::OnlyCircuit<decltype(p0)> p{p0};
    res = flex::weighted_matroid_intersection(g, p, w);
  }

  vector<i64> answer() const { return res.best; }
  vector<int> chosen() const { return res.set; }
};
