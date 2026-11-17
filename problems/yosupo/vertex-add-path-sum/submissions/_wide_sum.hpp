#pragma once
#include "_euler.hpp"
#include "_wide.hpp"

// 根からの和の差分の列を、節を 8 個に広げた接頭辞和の木 (_wide.hpp) で持つ。1 点の加算は段ごとに塊の 8 個へ AVX2 で足し、
// 接頭辞の和は段ごとに 1 つ読む (BIT は log N 回読む)。
struct WideSum {
  WidePrefixSum<3> w;
  WideSum(int, const vector<i64> &d) : w(d) {}
  void add(int i, i64 x) { w.add(i, x); }
  i64 prefix(int r) const { return w.prefix(r); }
};
