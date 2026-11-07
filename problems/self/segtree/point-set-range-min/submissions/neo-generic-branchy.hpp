#pragma once
#include "pj.hpp"
#include <limits>
#include "_seg_branchy.hpp"

// neo-generic と同じ自前の最小のモノイドを、区間の積を分岐ありにした汎用の形の写し (_seg_branchy.hpp) に渡す。
// neo-generic との差が、区間の積の分岐なしの集め方によるものかを見る。
struct Solver {
  struct RminQ {
    using T = i64;
    static T ti() { return numeric_limits<i64>::max(); }
    static T op(T a, T b) { return a < b ? a : b; }
  };
  SegA<RminQ> seg;
  explicit Solver(const vector<i64> &a) : seg(a) {}
  void set(int p, i64 x) { seg.set(p, x); }
  i64 minimum(int l, int r) { return seg.prod(l, r); }
};
