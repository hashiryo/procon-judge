#pragma once
// wm_sep と同じ形で、段ごとの累積を x の bit が 1 の段だけ読む (分岐で飛ばす) count_sum_less_br を使う。主記憶への
// 読み出しは半分になるが、x の bit で分岐する。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less_br(l, r, wavelet::lower(c.vals, x + 1)); }
};
