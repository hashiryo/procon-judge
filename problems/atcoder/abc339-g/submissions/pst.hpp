#pragma once
// 永続セグメント木 (problems/_shared/wavelet/pst.hpp)。節点に個数と値の和を持ち、x 以下の値の順位の上限の bit を
// たどって左の子の和を足す。x の順位は上位 16 bit の表 (Table) で引く。節点は huge page を頼んだ 1 本の配列。
#include "pj.hpp"
#include "_shared/wavelet/pst.hpp"
struct Solver {
 wavelet::Compressed c;
 vector<i64> w;
 wavelet::PersistentSegTree<true> t;
 wavelet::Table tbl;
 explicit Solver(const vector<int>& a): c(wavelet::compress(a)), w(a.begin(), a.end()), t(c.id, c.vals.size(), &w), tbl(c.vals) {}
 i64 sum_le(int l, int r, int x) const { return t.count_sum_less(l, r, tbl.lower_cm(x + 1)).second; }
};
