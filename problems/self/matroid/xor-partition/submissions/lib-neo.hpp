#pragma once
// NeoLibrary の matroid_intersection と、XorMatroid、PartitionMatroid を使う。
#include "neo/combinatorial_optimization/matroid_intersection.hpp"
#include "neo/combinatorial_optimization/matroids.hpp"

struct Solver {
  const vector<u64> &vec;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : vec(vec), color(color), cap(cap) {}

  void run() { ans = matroid_intersection(XorMatroid(vec), PartitionMatroid(color, cap)); }

  vector<int> answer() const { return ans; }
};
