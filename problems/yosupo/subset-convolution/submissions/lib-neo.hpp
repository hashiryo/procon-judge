#pragma once
// NeoLibrary の sps::convolve (neo/base/core/set_fps/convolve.hpp)。xw1 の法を実行時の奇数に広げたもの。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/convolve.hpp"
struct Solver {
 using Z= ZMod<998244353>;
 vector<Z> a, b, c;
 Solver(int, const vector<i64>& a_, const vector<i64>& b_): a(a_.size()), b(b_.size()) {
  for(size_t i= 0; i < a_.size(); ++i) a[i]= Z::raw(u32(a_[i])), b[i]= Z::raw(u32(b_[i]));
 }
 void run() { c= sps::convolve(a, b); }
 vector<i64> answer() const {
  vector<i64> r(c.size());
  for(size_t i= 0; i < c.size(); ++i) r[i]= c[i].val();
  return r;
 }
};
