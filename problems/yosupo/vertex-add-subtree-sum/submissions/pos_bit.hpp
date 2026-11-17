#pragma once
#include "_subtree.hpp"

// p_i < i を使って 2 回の走査で部分木が区間になる位置を配り (HLD はしない)、BIT を載せる。中身は _subtree.hpp。
struct Solver {
  subtree_ord::Buf pe;
  subtree_ord::Fenwick bit;
  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : pe(subtree_ord::layout<subtree_ord::Ord::Index>(n, par)), bit(n, pe.get(), a) {}
  void add(int u, i64 x) { bit.add(pe[2 * u], x); }
  i64 subtree_sum(int u) { return bit.sum(pe[2 * u], pe[2 * u + 1]); }
};
