#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の Sep の形。段ごとに 64 bit の語の列と、語ごとの累積の列を別に持つ。
// 値は基数ソートで順位に詰め、段の数は異なる値の数の bit 数。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WaveletMatrix<wavelet::Sep> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kth(l, r, k)]; }
};
