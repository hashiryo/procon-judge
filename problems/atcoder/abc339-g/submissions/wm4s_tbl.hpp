#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp) に、段ごとの記号 1 未満、2 未満、3 未満の値の累積を持たせた
// count_sum_less。x 以下の値の順位の上限は、上位 16 bit の表 (Table) で狭めてから lower_cm で探す。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WM4<true> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w), tbl(c.vals) {}
 i64 sum_le(int l, int r, int x) const { return wm.count_sum_less(l, r, tbl.lower_cm(x + 1)).second; }
};
