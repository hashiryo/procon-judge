#pragma once
// 8 分の wavelet matrix (problems/_shared/wavelet/wm8.hpp)。1 段で 3 bit ずつ分け、段の数を wm4 の 2/3 にする。段ごとに 64 個ずつ、
// 記号の 3 本の bit の列と記号 t 未満の累積を 64 byte に詰め、両端で記号 1 から 7 未満の数を 7 個ずつ求めて k と比べる。値は wm4 と
// 同じ基数ソートで順位に詰める。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM8 wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kth(l, r, k)]; }
};
