#pragma once
// NeoLibrary の sps::online_subset_zeta (neo/base/core/set_fps/relaxed.hpp) で解く。h[S] = phi(S, sum_{T ⊊ S} h[T]) の phi が
// dp[S] = f(c_S) (1 + 和) で、答えは dp の合計。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/set_fps/relaxed.hpp"
struct Solver {
 using Z= ZMod<1000000007>;
 int m;
 vector<int> cnt;
 u32 ans= 0;
 Solver(int m_, const vector<int>& cnt_): m(m_), cnt(cnt_) {}
 void run() {
  int mx= 0;
  for(int c: cnt) mx= max(mx, c);
  vector<Z> f(mx + 1);
  for(int c= 1; c <= mx; ++c) f[c]= Z(c) * (f[c - 1] + Z(1));
  const vector<Z> dp= sps::online_subset_zeta<Z>(m, [&](int S, Z& x) { x= f[cnt[S]] * (x + Z(1)); });
  Z t= Z();
  for(const Z& d: dp) t+= d;
  ans= t.val();
 }
 u32 answer() const { return ans; }
};
