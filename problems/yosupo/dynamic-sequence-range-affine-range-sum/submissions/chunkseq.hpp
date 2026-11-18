#pragma once
#include <array>
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "_shared/seqtree/chunk_seq.hpp"

// 列の木の汎用の形 (problems/_shared/seqtree/chunk_seq.hpp、節が値の配列を持つ treap) に、1 次関数と区間和の構造体を渡したもの。
// 塊の中は op と mp を 1 つずつ呼ぶ。塊の中を AVX2 で処理する chunk2 と比べて、速い核の分を見る。
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
  ChunkSeq<M, true> t;
  explicit Solver(const vector<i64> &a) : t(vector<Z>(a.begin(), a.end())) {}
  void insert(int i, i64 x) { t.insert(i, Z(x)); }
  void erase(int i) { t.erase(i); }
  void reverse(int l, int r) { t.reverse(l, r); }
  void affine(int l, int r, i64 b, i64 c) { t.apply(l, r, {Z(b), Z(c)}); }
  i64 sum(int l, int r) { return t.prod(l, r).val(); }
};
