#pragma once
// lib-neo の問い合わせの核 (NeoLibrary の WaveletMatrix<unsigned, false>) に、wm4 と同じ基数ソートの詰め方 (_shared/wavelet/wm.hpp の
// compress_s) と表 (Lookup) を組み合わせたもの。wm4_neoc と合わせて、lib-neo と wm4 の差が構築と問い合わせのどちらから来るかを分ける。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
#include "neo/data_structure/WaveletMatrix.hpp"
struct Solver {
 wavelet::Compressed c;
 WaveletMatrix<unsigned, false> wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& x): c(wavelet::compress_s(x)), wm(c.id), lk(c.vals) {}
 int count(int l, int r, int lo, int hi) const { return wm.count_lt(l, r, lk.lower(hi)) - wm.count_lt(l, r, lk.lower(lo)); }
};
