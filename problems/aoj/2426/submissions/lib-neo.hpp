#pragma once
// NeoLibrary の RectangleCount (neo/data_structure/RectangleCount.hpp)。点を x の順に並べた y の列の WaveletMatrix で数え、x の範囲は
// 値の範囲に合わせた表で位置の範囲に直す (wm4 と同じ形)。x の範囲を std::lower_bound で引いた形は、x64-gcc で wm4 の 2 倍ほど
// かかった (run 40477233432)。
#include "pj.hpp"
#include "neo/data_structure/RectangleCount.hpp"
struct Solver {
 RectangleCount<int> rc;
 static RectangleCount<int> make(const vector<array<int, 2>>& p) {
  vector<int> x(p.size()), y(p.size());
  for(size_t i= 0; i < p.size(); ++i) x[i]= p[i][0], y[i]= p[i][1];
  return RectangleCount<int>(x, y);
 }
 explicit Solver(const vector<array<int, 2>>& p): rc(make(p)) {}
 int count(int l, int d, int r, int u) const { return rc.count(l, r, d, u); }
};
