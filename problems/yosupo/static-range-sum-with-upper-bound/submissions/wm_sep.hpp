#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の Sep の形。段ごとに 64 bit の語の列と、語ごとの累積の列を別に持つ。
// 段ごとに、分けたあとの並びの値の累積も持つ。x 以下の値の順位の上限を二分探索で求め、その bit をたどりながら、
// bit が 1 の段で 0 の側へ行く値の数と和を足す。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less(l, r, wavelet::lower(c.vals, x + 1)); }
};
