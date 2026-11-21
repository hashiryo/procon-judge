#pragma once
// NeoLibrary の sps のゼータ (neo/base/core/set_fps/zeta.hpp)。t = 0 から 2 は標準の構造体 (monoid::Max、Min、Sum) を渡す
// モノイドのゼータ、3 から 5 は ZMod の環の変換で、どれも AVX2 の核に回る。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/ds/range/segtree/Min.hpp"
#include "neo/base/ds/range/segtree/Sum.hpp"
#include "neo/base/core/set_fps/zeta.hpp"
struct Solver {
  using Z = ZMod<998244353>;
  int t;
  vector<int> a32;
  vector<long long> a64;
  vector<unsigned long long> w;
  vector<Z> z;
  Solver(int t_, int, const vector<i64> &a) : t(t_) {
    if (t == 0) a32.assign(a.begin(), a.end());
    else if (t == 1) a64.assign(a.begin(), a.end());
    else if (t == 2) w.assign(a.begin(), a.end());
    else {
      z.resize(a.size());
      for (size_t i = 0; i < a.size(); ++i) z[i] = Z::raw(u32(a[i]));
    }
  }
  void run() {
    if (t == 0) sps::subset_zeta<monoid::Max<int>>(a32);
    else if (t == 1) sps::supset_zeta<monoid::Min<long long>>(a64);
    else if (t == 2) sps::subset_zeta<monoid::Sum<unsigned long long>>(w);
    else if (t == 3) sps::subset_zeta(z);
    else if (t == 4) sps::supset_mobius(z);
    else sps::hadamard(z);
  }
  vector<i64> answer() const {
    if (t == 0) return vector<i64>(a32.begin(), a32.end());
    if (t == 1) return a64;
    if (t == 2) return vector<i64>(w.begin(), w.end());
    vector<i64> r(z.size());
    for (size_t i = 0; i < z.size(); ++i) r[i] = z[i].val();
    return r;
  }
};
