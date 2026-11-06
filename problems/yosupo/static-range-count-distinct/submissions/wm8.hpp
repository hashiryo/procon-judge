#pragma once
// 同じ値が前に出た位置 + 1 の列 (_prev.hpp) の上に 8 分の wavelet matrix (problems/_shared/wavelet/wm8.hpp) を作り、[l, r) のうち
// l + 1 未満のものを数える。列の値は 0 以上 N 以下なので、順位に詰めずにそのまま使う。段の数が wm4 の 2/3。
#include "pj.hpp"
#include "_shared/wavelet/wm8.hpp"
#include "_prev.hpp"
struct Solver {
 wavelet::WM8 wm;
 explicit Solver(const vector<int>& a): wm(prev_plus_one(a), a.size() + 1) {}
 int count_distinct(int l, int r) const { return wm.count_less(l, r, l + 1); }
};
