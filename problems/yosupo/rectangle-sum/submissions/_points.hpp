#pragma once
// rectangle-sum の wm_* と pst が共有する前処理。点を x の順 (同じ x は入力の順) に並べ、並べた順の x、y の順位、
// 重みと、y の異なる値の昇順を作る。並べ替えと y の詰め方は problems/_shared/wavelet/wm.hpp の基数ソート。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Points {
 vector<int> xs, ys;
 vector<unsigned> yid;
 vector<i64> w;
 explicit Points(const vector<array<int, 3>>& p) {
  const int n= p.size();
  vector<int> x(n), y(n);
  for(int i= 0; i < n; ++i) x[i]= p[i][0];
  const auto k= wavelet::sorted_keys(x);
  xs.resize(n), w.resize(n);
  for(int j= 0; j < n; ++j) {
   const int i= int(unsigned(k[j]));
   xs[j]= p[i][0], y[j]= p[i][1], w[j]= p[i][2];
  }
  auto c= wavelet::compress(y);
  yid= std::move(c.id), ys= std::move(c.vals);
 }
};
