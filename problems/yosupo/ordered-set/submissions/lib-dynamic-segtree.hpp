#pragma once
// Library の SegmentTree_Dynamic (mylib/data_structure/SegmentTree_Dynamic.hpp) に、値の位置の個数の和を載せる。
// 高さ 30 の整数の 2 分木で、lib-patricia と違って道を縮めず、値ごとに 30 段の節点を持つ。値は 2^30 未満に限る。
// k 番目、x 以下の個数、前後の要素の引き方は lib-patricia と同じ。
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"
struct Solver {
 struct Sum {
  using T= int;
  static T ti() { return 0; }
  static T op(T a, T b) { return a + b; }
 };
 SegmentTree_Dynamic<Sum, false, 30> seg;
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
