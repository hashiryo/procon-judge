#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp)。1 段で 2 bit ずつ分け、段の数を 2 分の半分にする。段ごとに
// 64 個ずつ、記号の 2 本の bit の列と記号 0、1、2 の累積を 32 byte に詰め、区間の記号ごとの数を 1 回ずつの読み出しで
// 求める。値は wm_* と同じ基数ソートで順位に詰める。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WM4<> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), wm(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[wm.kth(l, r, k)]; }
};
