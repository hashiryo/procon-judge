#pragma once
// 2 分の wavelet matrix (problems/_shared/wavelet/wm.hpp の Sep)。値は負も受ける基数ソートで順位に詰め、境の順位は値の範囲に合わせた表 (Lookup) で引く。hi 未満の数から
// lo 未満の数を引く (2 回下りる)。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::WaveletMatrix<wavelet::Sep> wm;
 wavelet::Lookup lk;
 explicit Solver(const vector<int>& x): c(wavelet::compress_s(x)), wm(c.id, c.vals.size()), lk(c.vals) {}
 int count(int l, int r, int lo, int hi) const { return wm.count_less(l, r, lk.lower(hi)) - wm.count_less(l, r, lk.lower(lo)); }
};
