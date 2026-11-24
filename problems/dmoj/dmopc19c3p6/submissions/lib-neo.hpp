#pragma once
// NeoLibrary の matroid_union と GraphicMatroid (3 本の森の和) を使う。使った辺が 3(N - 1) 本なら、3 本とも全域木。
#include "neo/algo/optimize/matroid/matroid_union.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  vector<int> owner;
  bool good = false;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), e(edges) {}

  void run() {
    vector<GraphicMatroid> ms(3, GraphicMatroid(n, e));
    owner = matroid_union(ms);
    int used = 0;
    for (int g : owner) used += g >= 0;
    good = used == 3 * (n - 1);
  }

  bool ok() const { return good; }
  const vector<int> &group() const { return owner; }
};
