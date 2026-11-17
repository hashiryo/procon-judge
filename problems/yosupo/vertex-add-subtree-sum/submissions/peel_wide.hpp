#pragma once
#include "_subtree.hpp"
#include "_wide.hpp"

// 一般の木の HLD の位置 (子の数を数えて葉から剥がした順で作る) に、節を 8 個に広げた接頭辞和の木 (_wide.hpp、
// point-add-range-sum の wide8 の写し) を載せる。中身は _subtree.hpp。
struct Solver {
  subtree_ord::Buf pe;
  WidePrefixSum<3> w;
  static vector<i64> by_pos(int n, const int *pe, const vector<i64> &a) {
    vector<i64> v(n);
    for (int u = 0; u < n; ++u) v[pe[2 * u]] = a[u];
    return v;
  }
  Solver(int n, const vector<i64> &a, const vector<int> &par)
      : pe(subtree_ord::layout<subtree_ord::Ord::Peel>(n, par)), w(by_pos(n, pe.get(), a)) {}
  void add(int u, i64 x) { w.add(pe[2 * u], x); }
  i64 subtree_sum(int u) { return w.sum(pe[2 * u], pe[2 * u + 1]); }
};
