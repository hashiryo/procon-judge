#pragma once
// 永続セグメント木 (problems/_shared/wavelet/pst.hpp)。wavelet matrix と比べる相手。版 r と版 l を同時に下り、左の子の
// 個数の差で進む側を決める。節点は huge page を頼んだ 1 本の配列に取る。
#include "pj.hpp"
#include "_shared/wavelet/pst.hpp"
struct Solver {
 wavelet::Compressed c;
 wavelet::PersistentSegTree<false> t;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), t(c.id, c.vals.size()) {}
 int kth_smallest(int l, int r, int k) const { return c.vals[t.kth(l, r, k)]; }
};
