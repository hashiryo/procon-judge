#pragma once
#include "pj.hpp"
#include "mylib/data_structure/BinaryIndexedTree.hpp"

// 自分のライブラリの BinaryIndexedTree。fenwick.hpp が同じ構造の手書きなので、
// ライブラリを通したときに何が変わるかの比較になる。
struct Solver {
  BinaryIndexedTree<i64> bit;

  explicit Solver(const vector<i64> &init) : bit(init) {}

  void add(int p, i64 x) { bit.add(p, x); }

  i64 sum(int l, int r) { return bit.sum(l, r); }
};
