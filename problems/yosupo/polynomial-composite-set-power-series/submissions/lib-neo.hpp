#pragma once
// NeoLibrary の sps::poly_comp (neo/base/core/set_fps/composite.hpp)。xc1 の法を実行時の奇数に広げたもの。
// ZMod の列への写しと戻しは、構築と answer で行う (計測の外)。
#include "../common.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/composite.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 vector<Z> P, g, h;
 Solver(int, int, const vector<u32>& f, const vector<u32>& b): P(f.size()), g(b.size()) {
  for(size_t i= 0; i < f.size(); ++i) P[i]= Z::raw(f[i]);
  for(size_t i= 0; i < b.size(); ++i) g[i]= Z::raw(b[i]);
 }
 void run() { h= sps::poly_comp(P, g); }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
