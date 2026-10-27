#pragma once
// NeoLibrary の WaveletMatrixSum (neo/data_structure/WaveletMatrixSum.hpp)。点を x の順に並べ、y の列と重みの列から作り、
// x の範囲を std::lower_bound で位置の範囲に直して count_sum で引く (ドキュメントの使い方のとおり)。
#include <algorithm>
#include "pj.hpp"
#include "neo/data_structure/WaveletMatrixSum.hpp"
struct Solver {
 vector<int> xs;
 WaveletMatrixSum<int> wm;
 static vector<int> order(const vector<array<int, 3>>& p) {
  vector<int> o(p.size());
  for(size_t i= 0; i < p.size(); ++i) o[i]= i;
  sort(o.begin(), o.end(), [&](int i, int j) { return p[i][0] < p[j][0]; });
  return o;
 }
 Solver(const vector<array<int, 3>>& p, const vector<int>& o) {
  vector<int> ys(p.size());
  vector<i64> w(p.size());
  xs.resize(p.size());
  for(size_t j= 0; j < o.size(); ++j) xs[j]= p[o[j]][0], ys[j]= p[o[j]][1], w[j]= p[o[j]][2];
  wm= WaveletMatrixSum<int>(ys, w);
 }
 explicit Solver(const vector<array<int, 3>>& p): Solver(p, order(p)) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= lower_bound(xs.begin(), xs.end(), l) - xs.begin(), j= lower_bound(xs.begin(), xs.end(), r) - xs.begin();
  return wm.count_sum(i, j, d, u).second;
 }
};
