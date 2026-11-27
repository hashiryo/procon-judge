#pragma once
// NeoLibrary の sps_generic の 3^n の道 (semiring_convolve と semiring_exp) に、lib-neo と同じ構造を渡す。AVX2 の核に替える前の
// sps::convolve と sps::exp の中身で、lib-neo との差を見るための提出。
#include "pj.hpp"
#include "neo/base/core/algebra/semiring.hpp"
#include "neo/base/core/set_fps/convolve.hpp"
#include "neo/base/core/set_fps/exp.hpp"
struct Solver {
  using A = MinPlus<int>;
  using B = MaxPlus<long long>;
  using C = MaxMin<int>;
  static constexpr i64 NONE_MAX = -1000000000000000000LL;
  int t;
  vector<A::T> fa, ga, ha;
  vector<B::T> fb, gb, hb;
  vector<C::T> fc, gc, hc;
  Solver(int t_, int, const vector<i64> &f, const vector<i64> &g) : t(t_) {
    auto toA = [](i64 x) { return x == -1 ? A::zero() : int(x); };
    auto toB = [](i64 x) { return x == NONE_MAX ? B::zero() : (long long)x; };
    for (i64 x : f) t == 0 || t == 2 ? fa.push_back(toA(x)) : t == 4 ? fc.push_back(int(x)) : fb.push_back(toB(x));
    for (i64 x : g) t == 0 ? ga.push_back(toA(x)) : t == 4 ? gc.push_back(int(x)) : gb.push_back(toB(x));
  }
  void run() {
    if (t == 0) ha = sps_generic::semiring_convolve<A>(fa, ga);
    else if (t == 2) ha = sps_generic::semiring_exp<A>(fa);
    else if (t == 1) hb = sps_generic::semiring_convolve<B>(fb, gb);
    else if (t == 3) hb = sps_generic::semiring_exp<B>(fb);
    else hc = sps_generic::semiring_convolve<C>(fc, gc);
  }
  vector<i64> answer() const {
    vector<i64> r;
    for (auto x : ha) r.push_back(x == A::zero() ? -1 : x);
    for (auto x : hb) r.push_back(x == B::zero() ? NONE_MAX : x);
    for (auto x : hc) r.push_back(x);
    return r;
  }
};
