#pragma once
// 2 分の wavelet matrix (problems/_shared/wavelet/wm.hpp の Sep) に段ごとの値の累積を持たせた kth_sum。k 番目と同じく
// 下り、1 の側へ行く段で 0 の側へ行く値の和を足す (分岐で下りる)。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WaveletMatrix<wavelet::Sep, true> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w) {}
 i64 sum_smallest(int l, int r, int k) const { return wm.kth_sum(l, r, k); }
};
