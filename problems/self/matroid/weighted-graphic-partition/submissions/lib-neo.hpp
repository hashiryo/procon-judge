#pragma once
// NeoLibrary の weighted_matroid_intersection と、GraphicMatroid、PartitionMatroid を使う。
#include "neo/algo/optimize/matroid/matroid_intersection.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  const vector<i64> &w;
  vector<i64> best;
  vector<int> I;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap, const vector<i64> &w)
      : n(n), edges(edges), color(color), cap(cap), w(w) {}

  void run() { tie(best, I) = weighted_matroid_intersection(GraphicMatroid(n, edges), PartitionMatroid(color, cap), w); }

  vector<i64> answer() const { return best; }
  vector<int> chosen() const { return I; }
};
