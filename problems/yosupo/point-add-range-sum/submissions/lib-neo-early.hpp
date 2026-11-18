#pragma once
#include "_sum_exp.hpp"

// lib-neo-lean の prod を、l と r が同じ子に入った段で止める形 (SumExp<true>) にしたもの。この問題の区間は一様な乱択で
// ほとんど全部の段を読むので、止める判定の分だけ遅くならないかを見る。
struct Solver {
  SumExp<true> seg;
  explicit Solver(const vector<i64> &a) : seg((int)a.size(), [&](int i) { return a[i]; }) {}
  void add(int p, i64 x) { seg.mul(p, x); }
  i64 sum(int l, int r) { return seg.prod(l, r); }
};
