#pragma once
// NeoLibrary の matroid_union と XorMatroid を使う。
#include "neo/algo/optimize/matroid/matroid_union.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<XorMatroid> ms(k, XorMatroid(vec));
    ans = matroid_union(ms);
  }

  vector<int> answer() const { return ans; }
};
