#pragma once
// wm8 の k 番目を、区間の中の数を配列に書かずに比べる形 (problems/_shared/wavelet/wm8.hpp の kthp) にしたもの。wm8 の kth は
// 8 個の 32 bit の書き込みを 1 回のベクタで読み直していた。ほかは wm8 と同じ。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kthp(l, r, k)]; }
};
