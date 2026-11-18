#pragma once
#include <array>
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/seqtree/persistent_seq.hpp"

// 永続の列の木の試作 (problems/_shared/seqtree/persistent_seq.hpp、値を葉に持つ重み平衡木) を、永続しない形 (PERSIST = false) で
// 使ったもの。子へ遅延を配るときは子をその場で書き換え、split や併合で要らなくなった節は使い回す。1 節に 1 値の木として、今の
// Library の lib-wbt (同じく値を葉に持つ重み平衡木) と比べる。
struct Solver {
  using Z = ZMod<998244353>;
  struct M {
    using T = Z;
    using E = std::array<Z, 2>;  // x -> E[0] x + E[1]
    static T ti() { return T(); }
    static T op(T a, T b) { return a + b; }
    static void mp(T &v, const E &f, int len) { v = f[0] * v + f[1] * T(len); }
    static void cp(E &pre, const E &suf) { pre = {suf[0] * pre[0], suf[0] * pre[1] + suf[1]}; }
    using commute = void;
  };
  PersistentSeq<M, true, false> t;
  explicit Solver(const vector<i64> &a) : t(vector<Z>(a.begin(), a.end())) {}
  void insert(int i, i64 x) { t.insert(i, Z(x)); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
