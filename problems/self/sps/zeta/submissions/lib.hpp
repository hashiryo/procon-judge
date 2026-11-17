#pragma once
// 今の Library の sps (mylib/algebra/set_power_series.hpp)。ゼータは値の型の += を使うので、t = 0 と 1 は max と min を += に
// 書いた包みの型で呼ぶ (今の Library にモノイドのゼータは無い)。t = 2 は u64、3 から 5 は ModInt<998244353>。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/set_power_series.hpp"
struct MaxW {
  int v;
  MaxW &operator+=(const MaxW &o) { return v = v < o.v ? o.v : v, *this; }
};
struct MinW {
  long long v;
  MinW &operator+=(const MinW &o) { return v = o.v < v ? o.v : v, *this; }
};
struct Solver {
  using Mint = ModInt<998244353>;
  int t;
  vector<MaxW> a32;
  vector<MinW> a64;
  vector<unsigned long long> w;
  vector<Mint> z;
  Solver(int t_, int, const vector<i64> &a) : t(t_) {
    if (t == 0) {
      a32.resize(a.size());
      for (size_t i = 0; i < a.size(); ++i) a32[i].v = int(a[i]);
    } else if (t == 1) {
      a64.resize(a.size());
      for (size_t i = 0; i < a.size(); ++i) a64[i].v = a[i];
    } else if (t == 2) w.assign(a.begin(), a.end());
    else {
      z.resize(a.size());
      for (size_t i = 0; i < a.size(); ++i) z[i] = Mint(a[i]);
    }
  }
  void run() {
    if (t == 0) sps::subset_zeta(a32);
    else if (t == 1) sps::supset_zeta(a64);
    else if (t == 2) sps::subset_zeta(w);
    else if (t == 3) sps::subset_zeta(z);
    else if (t == 4) sps::supset_mobius(z);
    else sps::hadamard(z);
  }
  vector<i64> answer() const {
    vector<i64> r;
    if (t == 0)
      for (auto &x : a32) r.push_back(x.v);
    else if (t == 1)
      for (auto &x : a64) r.push_back(x.v);
    else if (t == 2) r.assign(w.begin(), w.end());
    else
      for (auto &x : z) r.push_back(i64(x.val()));
    return r;
  }
};
