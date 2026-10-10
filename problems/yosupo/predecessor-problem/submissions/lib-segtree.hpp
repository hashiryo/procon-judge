#pragma once
// Library の SegmentTree (mylib/data_structure/SegmentTree.hpp) に bool の or を載せる。k 以上で最小の要素は
// max_right(k, 区間が空か) の返す位置、k 以下で最大の要素は min_left(k + 1, 区間が空か) の 1 つ前。
#include "mylib/data_structure/SegmentTree.hpp"
struct Solver {
 struct Or {
  using T= bool;
  static T ti() { return false; }
  static T op(T a, T b) { return a | b; }
 };
 int n;
 SegmentTree<Or> seg;
 Solver(int n, const vector<u64>& bits): n(n), seg(n, [&](int i) -> bool { return bits[i >> 6] >> (i & 63) & 1; }) {}
 void insert(int k) { seg.set(k, true); }
 void erase(int k) { seg.set(k, false); }
 bool contains(int k) { return seg.get(k); }
 int next(int k) {
  const int r= seg.max_right(k, [](bool s) { return !s; });
  return r == n ? -1 : r;
 }
 int prev(int k) {
  return seg.min_left(k + 1, [](bool s) { return !s; }) - 1;
 }
};
