#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/FoldableQueue.hpp"

// NeoLibrary の FoldableQueue に、1 次関数の合成 (左を先に当てる) を自前の構造体で渡す。剰余は NeoLibrary の ZMod。
struct Solver {
  using Z = ZMod<998244353>;
  struct M {
    using T = array<Z, 2>;
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  };
  FoldableQueue<M> q;
  explicit Solver(int) {}
  void push(i64 a, i64 b) { q.push({Z(a), Z(b)}); }
  void pop() { q.pop(); }
  i64 query(i64 x) {
    const auto f = q.all_prod();
    return (f[0] * Z(x) + f[1]).val();
  }
};
