#pragma once
// NeoLibrary の matroid_union と GraphicMatroid を使う。
#include "neo/algo/optimize/matroid/matroid_union.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

struct Solver {
  int n, k;
  const vector<array<int, 2>> &edges;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), edges(edges) {}

  void run() {
    vector<GraphicMatroid> ms(k, GraphicMatroid(n, edges));
    ans = matroid_union(ms);
  }

  vector<int> answer() const { return ans; }
};
