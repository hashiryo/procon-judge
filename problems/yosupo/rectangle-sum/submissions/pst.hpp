#pragma once
// 永続セグメント木 (problems/_shared/wavelet/pst.hpp)。wavelet matrix と比べる相手。点を x の順に 1 個ずつ入れた版を
// 作り、節点に y の順位の範囲の個数と重みの和を持つ。x の範囲は並べた x の二分探索で版の範囲に直し、u 未満の和から
// d 未満の和を引く。節点は huge page を頼んだ 1 本の配列。
#include "pj.hpp"
#include "_shared/wavelet/pst.hpp"
#include "_points.hpp"
struct Solver {
 Points p;
 wavelet::PersistentSegTree<true> t;
 explicit Solver(const vector<array<int, 3>>& pts): p(pts), t(p.yid, p.ys.size(), &p.w) {}
 i64 rect_sum(int l, int d, int r, int u) const {
  const int i= wavelet::lower(p.xs, l), j= wavelet::lower(p.xs, r);
  return t.count_sum_less(i, j, wavelet::lower(p.ys, u)).second - t.count_sum_less(i, j, wavelet::lower(p.ys, d)).second;
 }
};
