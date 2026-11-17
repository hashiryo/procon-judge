#pragma once
// 切り分け: NeoLibrary の sps の k 進のゼータ変換とメビウス変換の、再帰しない形の写し (_flat/)。中身は lib-neo と同じ。比べ終えたら消す。
// t = 0、2、3 は環の形 (long long、ZMod、u64)、1 と 4 は標準の構造体 (monoid::Max、Min) のモノイドの形で、どれも AVX2 の核に回る。
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/data_structure/segtree/Min.hpp"
#include "_flat/zeta.hpp"
struct Solver {
  using Z = ZMod<998244353>;
  int t;
  vector<long long> a64;
  vector<int> a32;
  vector<unsigned long long> w;
  vector<Z> z;
  Solver(int t_, int, const vector<i64> &a) : t(t_) {
    if (t == 0 || t == 4) a64 = a;
    else if (t == 1) a32.assign(a.begin(), a.end());
    else if (t == 3) w.assign(a.begin(), a.end());
    else {
      z.resize(a.size());
      for (size_t i = 0; i < a.size(); ++i) z[i] = Z::raw(u32(a[i]));
    }
  }
  void run() {
    if (t == 0) sps::subset_zeta(a64, 10);
    else if (t == 1) sps::supset_zeta<monoid::Max<int>>(a32, 3);
    else if (t == 2) sps::subset_mobius(z, 4);
    else if (t == 3) sps::supset_zeta(w, 5);
    else sps::subset_zeta<monoid::Min<long long>>(a64, 7);
  }
  vector<i64> answer() const {
    if (t == 0 || t == 4) return a64;
    if (t == 1) return vector<i64>(a32.begin(), a32.end());
    if (t == 3) return vector<i64>(w.begin(), w.end());
    vector<i64> r(z.size());
    for (size_t i = 0; i < z.size(); ++i) r[i] = z[i].val();
    return r;
  }
};
