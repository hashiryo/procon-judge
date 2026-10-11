#pragma once
// wavelet matrix (problems/_shared/wavelet/wm.hpp) の B32 の形。段ごとに、32 bit の bit とその前までの累積を 1 つの 64 bit の語に詰める。
// 段ごとに、分けたあとの並びの値の累積も持つ。x 以下の値の順位の上限を二分探索で求め、その bit をたどりながら、
// bit が 1 の段で 0 の側へ行く値の数と和を足す。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::B32, true> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less(l, r, wavelet::lower(c.vals, x + 1)); }
};
