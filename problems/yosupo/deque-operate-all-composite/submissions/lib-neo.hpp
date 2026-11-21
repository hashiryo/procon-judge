#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/seq/FoldableDeque.hpp"

// NeoLibrary の FoldableDeque に、1 次関数の合成 (左を先に当てる) を自前の構造体で渡す。剰余は NeoLibrary の ZMod。構築で
// クエリの数だけ reserve しておく。
struct Solver {
  using Z = ZMod<998244353>;
  struct M {
    using T = array<Z, 2>;
    static T ti() { return {Z(1), Z(0)}; }
    static T op(const T &l, const T &r) { return {r[0] * l[0], r[0] * l[1] + r[1]}; }
  };
  FoldableDeque<M> q;
  explicit Solver(int n) { q.reserve(n); }  // 列はクエリの数より長くならない
  void push_front(i64 a, i64 b) { q.push_front({Z(a), Z(b)}); }
  void push_back(i64 a, i64 b) { q.push_back({Z(a), Z(b)}); }
  void pop_front() { q.pop_front(); }
  void pop_back() { q.pop_back(); }
  i64 query(i64 x) {
    const auto f = q.all_prod();
    return (f[0] * Z(x) + f[1]).val();
  }
};
