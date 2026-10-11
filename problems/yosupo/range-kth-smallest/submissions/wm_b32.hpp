#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の B32 の形。段ごとに、32 bit の bit とその前までの累積を 1 つの 64 bit の語に詰める。
// 値は基数ソートで順位に詰め、段の数は異なる値の数の bit 数。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WaveletMatrix<wavelet::B32> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kth(l, r, k)]; }
};
