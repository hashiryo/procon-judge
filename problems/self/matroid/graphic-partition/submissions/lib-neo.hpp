#pragma once
// NeoLibrary の matroid_intersection と、GraphicMatroid、PartitionMatroid を使う。
#include "neo/algo/optimize/matroid/matroid_intersection.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), edges(edges), color(color), cap(cap) {}

  void run() { ans = matroid_intersection(GraphicMatroid(n, edges), PartitionMatroid(color, cap)); }

  vector<int> answer() const { return ans; }
};
