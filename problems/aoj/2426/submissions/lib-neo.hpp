#pragma once
// NeoLibrary の WaveletMatrix (neo/data_structure/WaveletMatrix.hpp)。点を x の順に並べて y の列から作り、x の範囲を
// std::lower_bound で位置の範囲に直して count で数える (ドキュメントの使い方のとおり)。
#include <algorithm>
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 vector<int> xs;
 WaveletMatrix<int> wm;
 explicit Solver(const vector<array<int, 2>>& p) {
  vector<array<int, 2>> q(p);
  sort(q.begin(), q.end(), [](const array<int, 2>& a, const array<int, 2>& b) { return a[0] < b[0]; });
  vector<int> ys(q.size());
  xs.resize(q.size());
  for(size_t i= 0; i < q.size(); ++i) xs[i]= q[i][0], ys[i]= q[i][1];
  wm= WaveletMatrix<int>(ys);
 }
 int count(int l, int d, int r, int u) const {
  const int i= lower_bound(xs.begin(), xs.end(), l) - xs.begin(), j= lower_bound(xs.begin(), xs.end(), r) - xs.begin();
  return wm.count(i, j, d, u);
 }
};
