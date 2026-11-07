#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree.hpp"

// 今の Library には SWAG が無いので、足した順の位置ごとに 1 次関数を置く SegmentTree で書く。
// 末尾への追加は位置 tail への代入、先頭の削除は head を進めるだけ、取得は [head, tail) の積。
struct Solver {
  using Mint = ModInt<998244353>;
  struct Composite {
    using T = pair<Mint, Mint>;
    static T ti() { return {Mint(1), Mint(0)}; }
    static T op(const T &l, const T &r) { return {r.first * l.first, r.first * l.second + r.second}; }
  };
  SegmentTree<Composite> seg;
  int head = 0, tail = 0;

  explicit Solver(int q) : seg(q) {}
  void push(i64 a, i64 b) { seg.set(tail++, {Mint(a), Mint(b)}); }
  void pop() { ++head; }
  i64 query(i64 x) {
    const auto t = seg.prod(head, tail);
    return (t.first * Mint(x) + t.second).val();
  }
};
