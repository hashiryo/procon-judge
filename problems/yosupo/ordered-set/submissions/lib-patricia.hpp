#pragma once
// Library の SegmentTree_Patricia (mylib/data_structure/SegmentTree_Patricia.hpp) に、値の位置の個数の和を載せる。
// 高さ 30 の整数の 2 分木を道の途中で縮めたもので、値は 2^30 未満に限る。k 番目は find_first(0, 和 > k)、x 以下の個数は
// prod(0, x + 1)、x 以下で最大は find_last(x + 1, 和 > 0)、x 以上で最小は find_first(x, 和 > 0) で引く。
#include "mylib/data_structure/SegmentTree_Patricia.hpp"
struct Solver {
 struct Sum {
  using T= int;
  static T ti() { return 0; }
  static T op(T a, T b) { return a + b; }
 };
 SegmentTree_Patricia<Sum, false, 30> seg;
 explicit Solver(const vector<int>& a) {
  for(int x: a) seg.set(x, 1);
 }
 void insert(int x) { seg.set(x, 1); }
 void erase(int x) {
  if(!seg.is_null(x)) seg.set(x, 0);
 }
 int kth(int k) {
  return seg.find_first(0, [k](int s) { return s > k; });
 }
 int count_le(int x) { return seg.prod(0, x + 1); }
 int prev(int x) {
  return seg.find_last(x + 1, [](int s) { return s > 0; });
 }
 int next(int x) {
  return seg.find_first(x, [](int s) { return s > 0; });
 }
};
