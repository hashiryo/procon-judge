#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 今の Library の動的セグメント木に、range-affine-range-sum の lib-segtree-dynamic と同じ作用素モノイドを載せる。
// 添字の空間を 2^30 に取り、作用を配るときに無い子の節を new で作る。
struct Solver {
  using Mint = ModInt<998244353>;
  struct RaffineSum {
    using T = Mint;
    using E = array<Mint, 2>;  // {b, c} で x を b x + c にする
    static T ti() { return Mint(); }
    static T op(const T &l, const T &r) { return l + r; }
    // 和に作用させるので、区間の長さぶん c を足す。
    static void mp(T &v, const E &f, int sz) { v = f[0] * v + f[1] * sz; }
    static void cp(E &pre, const E &suf) { pre[0] *= suf[0], pre[1] = suf[0] * pre[1] + suf[1]; }
  };
  SegmentTree_Dynamic<RaffineSum, false, 30> seg;

  explicit Solver(int) {}

  void affine(int l, int r, i64 b, i64 c) { seg.apply(l, r, {Mint(b), Mint(c)}); }

  i64 sum(int l, int r) { return seg.prod(l, r).val(); }
};
