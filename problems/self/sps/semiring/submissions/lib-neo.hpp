#pragma once
// NeoLibrary の sps::convolve と sps::exp に、MinPlus<int>、MaxPlus<long long>、MaxMin<int> (neo/base/core/algebra/semiring.hpp) を渡す。
// どれも添字の下の bit を AVX2 のレーンに置く 3^n の核 (neo/base/core/set_fps/semiring_simd.hpp) に回る。
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
  vector<A> fa, ga, ha;
  vector<B> fb, gb, hb;
  vector<C> fc, gc, hc;
  Solver(int t_, int, const vector<i64> &f, const vector<i64> &g) : t(t_) {
    auto toA = [](i64 x) { return x == -1 ? A::zero() : A(int(x)); };
    auto toB = [](i64 x) { return x == NONE_MAX ? B::zero() : B(x); };
    for (i64 x : f) t == 0 || t == 2 ? fa.push_back(toA(x)) : t == 4 ? fc.push_back(C(int(x))) : fb.push_back(toB(x));
    for (i64 x : g) t == 0 ? ga.push_back(toA(x)) : t == 4 ? gc.push_back(C(int(x))) : gb.push_back(toB(x));
  }
  void run() {
    if (t == 0) ha = sps::convolve(fa, ga);
    else if (t == 2) ha = sps::exp(fa);
    else if (t == 1) hb = sps::convolve(fb, gb);
    else if (t == 3) hb = sps::exp(fb);
    else hc = sps::convolve(fc, gc);
  }
  vector<i64> answer() const {
    vector<i64> r;
    for (auto x : ha) r.push_back(x == A::zero() ? -1 : x.x);
    for (auto x : hb) r.push_back(x == B::zero() ? NONE_MAX : x.x);
    for (auto x : hc) r.push_back(x.x);
    return r;
  }
};
