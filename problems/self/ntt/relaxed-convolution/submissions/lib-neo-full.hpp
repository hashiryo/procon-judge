#pragma once
// NeoLibrary の relaxed::Convolution (f と g の両方を next で 1 つずつ渡す) を、既定の核 (Ntt) で回す。
// u32 から ZMod への変換は構築 (計測の外) で済ませる。
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/convolve/ntt.hpp"
#include "neo/base/core/poly/relaxed.hpp"

struct Solver {
 using Z= ZMod<998244353>;
 vector<Z> f, g;
 relaxed::Convolution<Z> rc;
 vector<u32> ans;

 static vector<Z> to_z(const vector<u32>& v) {
  vector<Z> r(v.size());
  for (size_t i= 0; i < v.size(); ++i) r[i]= Z::raw(v[i]);
  return r;
 }

 Solver(const vector<u32>& f_, const vector<u32>& g_): f(to_z(f_)), g(to_z(g_)), ans(f_.size() + g_.size() - 1) {}

 void run() {
  for (int i= 0; i < int(ans.size()); ++i) ans[i]= rc.next(i < int(f.size()) ? f[i] : Z(), i < int(g.size()) ? g[i] : Z()).val();
 }

 const vector<u32>& answer() const { return ans; }
};
