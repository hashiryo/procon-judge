#pragma once
#include "_subtree.hpp"

// 一般の木の HLD の位置 (子の数を数えて葉から剥がした順で作る) に BIT を載せる。中身は _subtree.hpp。
struct Solver {
  subtree_ord::Buf pe;
  subtree_ord::Fenwick bit;
  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : pe(subtree_ord::layout<subtree_ord::Ord::Peel>(n, par)), bit(n, pe.get(), a) {}
  void add(int u, i64 x) { bit.add(pe[2 * u], x); }
  i64 subtree_sum(int u) { return bit.sum(pe[2 * u], pe[2 * u + 1]); }
};
