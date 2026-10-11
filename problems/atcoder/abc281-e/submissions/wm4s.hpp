#pragma once
// 4 分の wavelet matrix (problems/_shared/wavelet/wm4.hpp) に、段ごとの記号 1 未満、2 未満、3 未満の値の累積を持たせた
// kth_sum。k 番目と同じく分岐なしで記号を選んで下り、記号が 0 でない段だけ、その記号未満の値の和を足す。
#include "pj.hpp"
#include "_shared/wavelet/wm4.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::WM4<true> wm;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), wm(c.id, c.vals.size(), &w) {}
 i64 sum_smallest(int l, int r, int k) const { return wm.kth_sum(l, r, k); }
};
