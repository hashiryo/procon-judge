#pragma once
// NeoLibrary の sps のゼータの汎用の道。t = 0 から 2 は標準の構造体と同じ演算の自前の構造体を渡して、op を 1 つずつ呼ぶ
// 汎用の道に回し、3 から 5 は sps_generic (今の Library の写し) を呼ぶ。AVX2 の核 (lib-neo) との差を見るための提出。
#include <climits>
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/zeta.hpp"
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
struct MySum {
  using T = unsigned long long;
  static T ti() { return 0; }
  static T op(T a, T b) { return a + b; }
};
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
    if (t == 0) sps::subset_zeta<MyMax>(a32);
    else if (t == 1) sps::supset_zeta<MyMin>(a64);
    else if (t == 2) sps::subset_zeta<MySum>(w);
    else if (t == 3) sps_generic::subset_zeta(z);
    else if (t == 4) sps_generic::supset_mobius(z);
    else sps_generic::hadamard(z);
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
