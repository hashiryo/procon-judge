#pragma once
// wm_sep_br (累積を x の bit が 1 の段だけ読む) と、x 以下の値の順位の上限を上位 16 bit の表で狭めてから lower_cm で
// 探す形を合わせたもの。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w), tbl(c.vals) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less_br(l, r, tbl.lower_cm(x + 1)); }
};
