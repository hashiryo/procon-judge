#pragma once
// 2 分の wavelet matrix (problems/_shared/wavelet/wm.hpp の Sep) に段ごとの値の累積を持たせ、累積を x の bit が 1 の
// 段だけ読む count_sum_less_br。x 以下の値の順位の上限は、上位 16 bit の表 (Table) で狭めてから lower_cm で探す。
// yosupo-static-range-sum-with-upper-bound でいちばん速かった形。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w), tbl(c.vals) {}
 i64 sum_le(int l, int r, int x) const { return wm.count_sum_less_br(l, r, tbl.lower_cm(x + 1)).second; }
};
