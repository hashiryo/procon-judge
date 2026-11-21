#pragma once
// NeoLibrary の sps::power_projection (neo/algo/count/set_fps/power_projection.hpp)。xp1 の法を実行時の奇数に広げ、
// 答えの段を割り算を使わない漸化式にしたもの。ZMod の列への写しと戻しは、構築と answer で行う (計測の外)。
#include "../common.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/count/set_fps/power_projection.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 int M;
 vector<Z> f, x, h;
 Solver(int, int M_, const vector<u32>& a, const vector<u32>& w): M(M_), f(a.size()), x(w.size()) {
  for(size_t i= 0; i < a.size(); ++i) f[i]= Z::raw(a[i]), x[i]= Z::raw(w[i]);
 }
 void run() { h= sps::power_projection(f, x, M); }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
