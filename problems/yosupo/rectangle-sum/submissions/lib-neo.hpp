#pragma once
// NeoLibrary の RectangleSum (neo/data_structure/RectangleSum.hpp)。点を x の順に並べた y と重みの列の WaveletMatrixSum で引き、
// x の範囲は値の範囲に合わせた表で位置の範囲に直す (wm_sep_brtbl と同じ形)。x の範囲を std::lower_bound で引いた形は、9V45 の
// x64-gcc で wm_sep_brtbl の 1.17 倍かかった (run 40482922246)。
#include "pj.hpp"
#include "neo/data_structure/RectangleSum.hpp"
struct Solver {
 RectangleSum<int> rs;
 static RectangleSum<int> make(const vector<array<int, 3>>& p) {
  vector<int> x(p.size()), y(p.size());
  vector<i64> w(p.size());
  for(size_t i= 0; i < p.size(); ++i) x[i]= p[i][0], y[i]= p[i][1], w[i]= p[i][2];
  return RectangleSum<int>(x, y, w);
 }
 explicit Solver(const vector<array<int, 3>>& p): rs(make(p)) {}
 i64 rect_sum(int l, int d, int r, int u) const { return rs.sum(l, r, d, u); }
};
