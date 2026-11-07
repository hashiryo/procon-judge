#pragma once
#include "pj.hpp"
#include "mylib/data_structure/BinaryIndexedTree_RangeAdd.hpp"

// 今の Library の BinaryIndexedTree_RangeAdd (差分を 2 本の BIT に持つ形)。初めの値は 1 点ずつの区間加算で入れる。
struct Solver {
  BinaryIndexedTree_RangeAdd<i64> bit;
  explicit Solver(const vector<i64> &a) : bit((int)a.size()) {
    for (int i = 0; i < (int)a.size(); ++i) bit.add_range(i, i + 1, a[i]);
  }
  void add(int l, int r, i64 x) { bit.add_range(l, r, x); }
  i64 sum(int l, int r) { return bit.sum(l, r); }
};
