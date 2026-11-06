#pragma once
// wm4 の値の詰め方だけを NeoLibrary の compress (neo/sort/compress.hpp) に替えたもの。lib-neo が wm4 より 1 割ほど遅い理由が、
// 構築の圧縮か、問い合わせの道かを分けるために置く (問い合わせは wm4 と同じ)。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
#include "neo/sort/compress.hpp"
struct Solver {
 vector<int> vals;
 vector<unsigned> id;
 wavelet::WM4<> wm;
 wavelet::Lookup lk;
 static vector<unsigned> ranks(const vector<int>& x, vector<int>& vals) {
  vector<int> c(x);
  vals= compress(c);
  return vector<unsigned>(c.begin(), c.end());
 }
 explicit Solver(const vector<int>& x): id(ranks(x, vals)), wm(id, vals.size()), lk(vals) {}
 int count(int l, int r, int lo, int hi) const { return wm.count_less(l, r, lk.lower(hi)) - wm.count_less(l, r, lk.lower(lo)); }
};
