#pragma once
// NeoLibrary の sps::online_subset_zeta (neo/base/core/set_fps/relaxed.hpp) で解く。値は (dp, dp2) の組で、A (s の部分集合の数) は
// sps::subset_zeta (neo/base/core/set_fps/zeta.hpp) で求める。
// dp[k] = dp2[k - 1] (dp[0] = 1)、dp2[k] = A(k) (sum_{j ⊊ k} dp[j] + dp[k]) - sum_{j ⊊ k} dp2[j]。答えは dp2[2^N - 1]。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/relaxed.hpp"
#include "neo/base/core/set_fps/zeta.hpp"
struct Solver {
 using Z= ZMod<998244353>;
 struct Pair {
  Z a, b;
  Pair& operator+=(const Pair& o) {
   a+= o.a, b+= o.b;
   return *this;
  }
 };
 int n;
 vector<int> s;
 u32 ans= 0;
 Solver(int n_, const vector<int>& s_): n(n_), s(s_) {}
 void run() {
  vector<u32> A(size_t(1) << n, 0);
  for(int x: s) A[x]= 1;
  sps::subset_zeta(A);
  Z prev= Z(1);  // dp[0] = 1
  sps::online_subset_zeta<Pair>(n, [&](int k, Pair& x) {
   const Z dp= prev, dp2= Z(A[k]) * (x.a + dp) - x.b;
   x= Pair{dp, dp2};
   prev= dp2;
  });
  ans= prev.val();
 }
 u32 answer() const { return ans; }
};
