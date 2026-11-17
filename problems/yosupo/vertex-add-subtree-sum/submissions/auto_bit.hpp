#pragma once
#include "_subtree.hpp"

// 一般の木の HLD の位置に BIT を載せる (中身は _subtree.hpp)。先に p_i < i (根が 0) かを確かめ、そうなら番号の大きい順を
// そのまま使って位置を配り、違えば peel_bit と同じく葉から剥がす。
struct Solver {
  subtree_ord::Buf pe;
  subtree_ord::Fenwick bit;
  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : pe(subtree_ord::layout<subtree_ord::Ord::Auto>(n, par)), bit(n, pe.get(), a) {}
  void add(int u, i64 x) { bit.add(pe[2 * u], x); }
  i64 subtree_sum(int u) { return bit.sum(pe[2 * u], pe[2 * u + 1]); }
};
