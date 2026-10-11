#pragma once
// 2 分の wavelet matrix (problems/_shared/wavelet/wm.hpp の Sep) を、x の順に並べた点の y の順位の列の上に作る。x の範囲と y の境の順位は、値の範囲に合わせた表 (Lookup) で
// 引き、u 未満の数から d 未満の数を引く (2 回下りる)。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WaveletMatrix<wavelet::Sep> wm;
 wavelet::Lookup tx, ty;
 explicit Solver(const vector<array<int, 2>>& pts): p(pts), wm(p.yid, p.ys.size()), tx(p.xs), ty(p.ys) {}
 int count(int l, int d, int r, int u) const {
  const int i= tx.lower(l), j= tx.lower(r);
  return wm.count_less(i, j, ty.lower(u)) - wm.count_less(i, j, ty.lower(d));
 }
};
