#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp) の count_sum_less。段ごとに、記号が 1 未満、2 未満、3 未満の
// 値の累積を 3 本持ち、x の記号が 0 でない段だけその記号の 1 本を両端で読む。段の数が 2 分の半分になる代わりに、
// 累積の大きさは 2 分の 1.5 倍。x 以下の値の順位の上限は、上位 16 bit の表で狭めてから lower_cm で探す。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WM4<true> wm;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w), tbl(c.vals) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return wm.count_sum_less(l, r, tbl.lower_cm(x + 1)); }
};
