#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library には SWAG が無いので、位置ごとに 1 次関数を置く SegmentTree で書く。位置は q を
// 起点に、先頭への追加で 1 つ前、末尾への追加で 1 つ後ろへ伸ばす。取得は [head, tail) の積。
struct Solver {
  using Mint = ModInt<998244353>;
  struct Composite {
    using T = pair<Mint, Mint>;
    static T ti() { return {Mint(1), Mint(0)}; }
    static T op(const T &l, const T &r) { return {r.first * l.first, r.first * l.second + r.second}; }
  };
  SegmentTree<Composite> seg;
  int head, tail;

  explicit Solver(int q) : seg(2 * q + 1), head(q), tail(q) {}
  void push_front(i64 a, i64 b) { seg.set(--head, {Mint(a), Mint(b)}); }
  void push_back(i64 a, i64 b) { seg.set(tail++, {Mint(a), Mint(b)}); }
  void pop_front() { ++head; }
  void pop_back() { --tail; }
  i64 query(i64 x) {
    const auto t = seg.prod(head, tail);
    return (t.first * Mint(x) + t.second).val();
  }
};
