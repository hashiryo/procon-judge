#pragma once
#include "pj.hpp"
#include "mylib/optimization/WeightedMatching.hpp"

// 一般グラフの重み付きマッチング。二部であることを使わず、花を潰しながら
// 増加路を探す。二部専用の手に対する上界になる。
struct Solver {
  int n;
  vector<i64> a;
  i64 total = 0;
  vector<int> p;

  Solver(int n, const vector<i64> &a) : n(n), a(a) {}

  void run() {
    WeightedMatching<i64, true> g(n + n);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j) g.add_edge(i, n + j, a[(size_t)i * n + j]);
    g.build();
    total = 0;
    for (auto [u, v, w] : g.weight_matching()) total += w;
    p.resize(n);
    for (int i = 0; i < n; ++i) p[i] = g.match(i) - n;
  }

  i64 cost() const { return total; }

  const vector<int> &assignment() const { return p; }
};
