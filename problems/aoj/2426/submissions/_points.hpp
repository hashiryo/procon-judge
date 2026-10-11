#pragma once
// aoj-2426 の wm4 と wm_sep が共有する前処理。点を x の順 (同じ x は入力の順) に並べ、並べた順の x と、y の順位と、
// y の異なる値の昇順を作る。並べ替えと y の詰め方は problems/_shared/wavelet/wm.hpp の、負の値も受ける基数ソート。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Points {
 vector<int> xs, ys;
 vector<unsigned> yid;
 explicit Points(const vector<array<int, 2>>& p) {
  const int n= p.size();
  vector<int> x(n), y(n);
  for(int i= 0; i < n; ++i) x[i]= p[i][0];
  const auto k= wavelet::sorted_keys_s(x);
  xs.resize(n);
  for(int j= 0; j < n; ++j) {
   const int i= int(unsigned(k[j]));
   xs[j]= p[i][0], y[j]= p[i][1];
  }
  auto c= wavelet::compress_s(y);
  yid= std::move(c.id), ys= std::move(c.vals);
 }
};
