#pragma once
// wm_sep の x 以下の値の順位の上限を、上位 16 bit の表 (problems/_shared/wavelet/wm.hpp の Table) で区間に狭めてから
// 探す形。累積の読み方は wm_sep と同じ (全段で読んでマスクで捨てる)。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w), tbl(c.vals) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less(l, r, tbl.lower(x + 1)); }
};
