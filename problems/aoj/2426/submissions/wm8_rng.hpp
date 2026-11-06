#pragma once
// wm8 の y の範囲の個数を、2 つの境の記号が同じ段を 1 回で下り、違う段から下を 2 本同じループで下りる形
// (problems/_shared/wavelet/wm8.hpp の count_range) にしたもの。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::WM8 wm;
 wavelet::Lookup tx, ty;
 explicit Solver(const vector<array<int, 2>>& pts): p(pts), wm(p.yid, p.ys.size()), tx(p.xs), ty(p.ys) {}
 int count(int l, int d, int r, int u) const {
  const int i= tx.lower(l), j= tx.lower(r), a= ty.lower(d), b= ty.lower(u);
  return a < b ? wm.count_range(i, j, a, b) : 0;
 }
};
