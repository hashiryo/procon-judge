#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_deque_exp.hpp"

// NeoLibrary の FoldableDeque の写し (_deque_exp.hpp) で、伸ばすときに新しい配列を埋めず、生きている要素だけを写す形 (1 MB 以上は huge page のまま)。値と演算は lib-neo と同じ。
struct Solver {
  using Z = ZMod<998244353>;
  struct M {
    using T = array<Z, 2>;
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  };
  FoldableDequeExp<M, false, true> dq;
  explicit Solver(int) {}
  void push_front(i64 a, i64 b) { dq.push_front({Z(a), Z(b)}); }
  void push_back(i64 a, i64 b) { dq.push_back({Z(a), Z(b)}); }
  void pop_front() { dq.pop_front(); }
  void pop_back() { dq.pop_back(); }
  i64 query(i64 x) {
    const auto f = dq.all_prod();
    return (f[0] * Z(x) + f[1]).val();
  }
};
