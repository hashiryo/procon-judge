#pragma once
// NeoLibrary の sps の k 進の変換の汎用の道。値を包みの型 (+= と -= だけを持つ) や自前の構造体にして、AVX2 の核に乗せずに
// 値を 1 つずつ合わせる道に回す。AVX2 の核 (lib-neo) との差を見るための提出。
#include <climits>
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/zeta.hpp"
template <class V> struct Wrap {
  V v;
  Wrap &operator+=(const Wrap &o) { return v += o.v, *this; }
  Wrap &operator-=(const Wrap &o) { return v -= o.v, *this; }
};
struct MyMax {
  using T = int;
  static T ti() { return INT_MIN; }
  static T op(T a, T b) { return a < b ? b : a; }
};
struct MyMin {
  using T = long long;
  static T ti() { return LLONG_MAX; }
  static T op(T a, T b) { return b < a ? b : a; }
};
struct Solver {
  using Z = ZMod<998244353>;
  int t;
  vector<Wrap<long long>> s64;
  vector<long long> m64;
  vector<int> a32;
  vector<Wrap<unsigned long long>> w;
  vector<Wrap<Z>> z;
  Solver(int t_, int, const vector<i64> &a) : t(t_) {
    for (i64 x : a) {
      if (t == 0) s64.push_back({x});
      else if (t == 1) a32.push_back(int(x));
      else if (t == 2) z.push_back({Z::raw(u32(x))});
      else if (t == 3) w.push_back({u64(x)});
      else m64.push_back(x);
    }
  }
  void run() {
    if (t == 0) sps::subset_zeta(s64, 10);
    else if (t == 1) sps::supset_zeta<MyMax>(a32, 3);
    else if (t == 2) sps::subset_mobius(z, 4);
    else if (t == 3) sps::supset_zeta(w, 5);
    else sps::subset_zeta<MyMin>(m64, 7);
  }
  vector<i64> answer() const {
    vector<i64> r;
    for (auto &x : s64) r.push_back(x.v);
    for (int x : a32) r.push_back(x);
    for (auto &x : z) r.push_back(x.v.val());
    for (auto &x : w) r.push_back(i64(x.v));
    for (long long x : m64) r.push_back(x);
    return r;
  }
};
