#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_deque_exp.hpp"

// NeoLibrary の FoldableDeque の写し (_deque_exp.hpp) で、写し元と同じ取り方で、構築のときに容量 2 q + 16 の配列を取って埋めておく形 (計測区間の中で伸ばさない)。値と演算は lib-neo と同じ。
struct Solver {
  using Z = ZMod<998244353>;
  struct M {
    using T = array<Z, 2>;
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  };
  FoldableDequeExp<M, true, true> dq;
  explicit Solver(int q) { dq.reserve(2 * q + 16); }
  void push_front(i64 a, i64 b) { dq.push_front({Z(a), Z(b)}); }
  void push_back(i64 a, i64 b) { dq.push_back({Z(a), Z(b)}); }
  void pop_front() { dq.pop_front(); }
  void pop_back() { dq.pop_back(); }
  i64 query(i64 x) {
    const auto f = dq.all_prod();
    return (f[0] * Z(x) + f[1]).val();
  }
};
