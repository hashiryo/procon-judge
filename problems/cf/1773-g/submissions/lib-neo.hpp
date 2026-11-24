#pragma once
// NeoLibrary の sps::online_subset_zeta (neo/base/core/set_fps/relaxed.hpp) で解く。Genie を除いた m - 1 人の集合 S' の補集合 U の小さい順に、
// dp[S'] (S' に着く確率) を求める。Genie が正解する質問 (g = 1) の q' (Genie を除いた集合) で S' → S' ∩ q' (S' ⊄ q') と進むので、
// U ∪ V の OR 畳み込み (W[U] = dp / eff、C[V] = cnt1[~V]、重なりを許す) から S' = T' の項を除いたものが dp[T']。
//   dp[U] = C^[U] sum_{X ⊊ U} W[X] - sum_{X ⊊ U} H[X] (U = 0 は 1)、H[X] = dp[X] - [X = 0] + W[X] C^[X]、W = dp / eff
// (C^[U] = #{g = 1、q' ⊇ S'} は上位集合の和)。値は (W, H) の組で、phi が受け取る和は真の部分集合の (sum W, sum H)。引き算で桁が落ちるので
// long double で持つ。eff(S') = #{g = 1、q' ⊉ S'} + #{g = 0、q' ∩ S' ≠ ∅}。eff = 0 の S' の dp を答えに足す。
#include "pj.hpp"
#include "neo/base/core/set_fps/relaxed.hpp"
#include "neo/base/core/set_fps/zeta.hpp"
struct Solver {
 using R= long double;
 struct Pair {
  R w, h;
  Pair& operator+=(const Pair& o) {
   w+= o.w, h+= o.h;
   return *this;
  }
 };
 int n, m;
 vector<u32> q;
 double ans= 0;
 Solver(int n_, int m_, const vector<u32>& q_): n(n_), m(m_), q(q_) {}
 void run() {
  const int mm= m - 1, full= (1 << mm) - 1;
  vector<int> c1(size_t(1) << mm, 0), c0(size_t(1) << mm, 0);
  int N1= 0, N0= 0;
  for(u32 x: q) {
   if(x & 1) ++c1[x >> 1], ++N1;
   else ++c0[x >> 1], ++N0;
  }
  sps::supset_zeta(c1), sps::subset_zeta(c0);  // c1[S'] = #{g = 1、q' ⊇ S'}、c0[U] = #{g = 0、q' ⊆ U}
  R total= 0;
  sps::online_subset_zeta<Pair>(mm, [&](int U, Pair& x) {
   const int S= full ^ U;
   const R chat= c1[S];
   const R dp= U == 0 ? R(1) : chat * x.w - x.h;
   const int eff= (N1 - c1[S]) + (N0 - c0[U]);
   R W= 0;
   if(eff == 0) total+= dp;
   else W= dp / eff;
   x= Pair{W, dp - (U == 0 ? R(1) : R(0)) + W * chat};
  });
  ans= double(total);
 }
 double answer() const { return ans; }
};
