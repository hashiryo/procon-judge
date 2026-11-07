#pragma once
// NeoLibrary の PointAddRectangleSum (neo/data_structure/PointAddRectangleSum.hpp)。最初の点と後から足される点を全部まとめて作り
// (後から足す点は重み 0)、k 番目の add は、構築で受け取った k 番目の座標の点の添字に足す。この問題で書き比べた wm_bit を写したもの。
#include "pj.hpp"
#include "neo/data_structure/PointAddRectangleSum.hpp"
struct Solver {
 int base= 0, adds= 0;
 PointAddRectangleSum<int> ps;
 static PointAddRectangleSum<int> make(const vector<array<i64, 3>>& pts, const vector<array<int, 2>>& spots) {
  const int N= pts.size(), n= N + spots.size();
  vector<int> x(n), y(n);
  vector<i64> w(n, 0);
  for(int i= 0; i < N; ++i) x[i]= int(pts[i][0]), y[i]= int(pts[i][1]), w[i]= pts[i][2];
  for(size_t k= 0; k < spots.size(); ++k) x[N + k]= spots[k][0], y[N + k]= spots[k][1];
  return PointAddRectangleSum<int>(x, y, w);
 }
 Solver(const vector<array<i64, 3>>& pts, const vector<array<int, 2>>& spots): base(pts.size()), ps(make(pts, spots)) {}
 void add(int, int, i64 w) { ps.add(base + adds++, w); }
 i64 rect_sum(int l, int d, int r, int u) const { return ps.sum(l, r, d, u); }
};
