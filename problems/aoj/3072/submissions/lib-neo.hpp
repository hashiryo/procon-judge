#pragma once
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/poly/sparse_fps.hpp"
// NeoLibrary の sparse_fps::div。分母の 0 でない項が 3 つしかないことを使って、漸化式を直接回す O(K)。長さ k を渡す。
struct Solver {
 int n, k, p;
 i64 res= 0;
 Solver(int n, int k, int p): n(n), k(k), p(p) {}
 void run() {
  using Z= ZMod<998244353>;
  Z pr= Z(p) / 100, c= (Z(1) - pr) / pr;
  vector<Z> f(n + 2);
  f[0]= n, f[1]= -(pr + n), f[n + 1]= pr;
  auto g= sparse_fps::div<Z>({c * n, -c * n}, f, k);
  Z ans= 1;
  for(int i= 1; i < k; ++i) ans-= g[i];
  res= ans.val();
 }
 i64 answer() const { return res; }
};
