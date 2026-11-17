#pragma once
// NeoLibrary の sps::log (neo/set_power_series/log.hpp)。xl1 の法を実行時の奇数に広げたもの。
// ZMod の列への写しと戻しは、構築と answer で行う (計測の外)。
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/log.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 vector<Z> f, h;
 Solver(int, const vector<u32>& b): f(b.size()) {
  for(size_t i= 0; i < b.size(); ++i) f[i]= Z::raw(b[i]);
 }
 void run() { h= sps::log(f); }
 vector<u32> answer() const {
  vector<u32> r(h.size());
  for(size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
