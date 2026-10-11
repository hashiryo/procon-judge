#pragma once
// 永続セグメント木 (problems/_shared/wavelet/pst.hpp) の kth_sum_v。版 r と版 l を同時に下り、右へ行く段で左の子の
// 和を足し、葉に残った k 個は値の k 倍を足す。節点は huge page を頼んだ 1 本の配列。
#include "pj.hpp"
#include "_shared/wavelet/pst.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::PersistentSegTree<true> t;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), t(c.id, c.vals.size(), &w) {}
 i64 sum_smallest(int l, int r, int k) const { return t.kth_sum_v(l, r, k, c.vals); }
};
