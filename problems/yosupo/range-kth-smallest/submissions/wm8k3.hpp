#pragma once
// wm8 の k 番目を、段の中の 3 bit を上から 1 bit ずつ選ぶ形 (problems/_shared/wavelet/wm8.hpp の kth3) にしたもの。段ごとの
// popcount は 6 回で、スタックの配列を使わない。ほかは wm8 と同じ。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kth3(l, r, k)]; }
};
