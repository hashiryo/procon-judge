#pragma once
// 点が 5000 個以下なので、x と y をそれぞれ異なる値に詰めた格子の上に 2 次元の累積和を作る (最大 5001 x 5001 の
// int で 100 MB)。長方形は、境の順位を値の範囲に合わせた表 (Lookup) で引いて 4 か所の差を取る。wavelet matrix の
// 要らない大きさで、どこまで差が出るかを見る相手。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 vector<int> xv, yv;
 wavelet::Lookup tx, ty;
 int W= 0;
 vector<int> g;
 explicit Solver(const vector<array<int, 2>>& p) {
  const int n= p.size();
  vector<int> x(n), y(n);
  for(int i= 0; i < n; ++i) x[i]= p[i][0], y[i]= p[i][1];
  auto cx= wavelet::compress_s(x), cy= wavelet::compress_s(y);
  xv= std::move(cx.vals), yv= std::move(cy.vals);
  tx= wavelet::Lookup(xv), ty= wavelet::Lookup(yv);
  const int H= xv.size() + 1;
  W= yv.size() + 1;
  g.assign(size_t(H) * W, 0);
  for(int i= 0; i < n; ++i) ++g[size_t(cx.id[i] + 1) * W + cy.id[i] + 1];
  for(int a= 1; a < H; ++a)
   for(int b= 1; b < W; ++b) g[size_t(a) * W + b]+= g[size_t(a - 1) * W + b] + g[size_t(a) * W + b - 1] - g[size_t(a - 1) * W + b - 1];
 }
 int count(int l, int d, int r, int u) const {
  const int i= tx.lower(l), j= tx.lower(r), a= ty.lower(d), b= ty.lower(u);
  return g[size_t(j) * W + b] - g[size_t(i) * W + b] - g[size_t(j) * W + a] + g[size_t(i) * W + a];
 }
};
