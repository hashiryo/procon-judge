#pragma once
// 永続セグメント木 (problems/_shared/wavelet/pst.hpp)。wavelet matrix と比べる相手。節点に個数と値の和を持ち、x 以下の
// 値の順位の上限の bit をたどりながら、bit が 1 の段で左の子の個数と和を足す。節点は huge page を頼んだ 1 本の配列。
#include "pj.hpp"
#include "_shared/wavelet/pst.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::PersistentSegTree<true> t;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), t(c.id, c.vals.size(), &w) {}
 pair<int, i64> count_sum(int l, int r, int x) const { return t.count_sum_less(l, r, wavelet::lower(c.vals, x + 1)); }
};
