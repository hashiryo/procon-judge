#pragma once
// 書き比べ: h[S] = phi(S, sum_{T ⊊ S} h[T]) を S の小さい順に求める関数の形 (オンラインのゼータ) で解く。値は (dp, dp2) の組。
// 区画 [lo, lo + 2^b) の再帰で、Y は入るときに off[t] = sum_{j < lo, j ⊆ lo + t} h[j]、出るときに区画の中の h のゼータ変換を持つ。
// 下の半分を再帰で済ませると下の半分は下の半分のゼータになるので、それを上の半分の off に足して上の半分を再帰で済ませ、最後に
// もう一度足すと全体のゼータになる。足し算はどれも連続した配列どうしで、全体で n 2^n 回。大きさ 8 の区画は素朴に求める。
// dp[k] = dp2[k - 1] (dp[0] = 1)、dp2[k] = A(k) (sum_{j ⊊ k} dp[j] + dp[k]) - sum_{j ⊊ k} dp2[j]。答えは dp2[2^N - 1]。
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
template <class T, class P> void ozeta_rec(T* Y, size_t lo, int b, const P& phi) {
 if(b <= 3) {
  const int w= 1 << b;
  T h[8];
  for(int t= 0; t < w; ++t) {
   T x= Y[t];
   for(int u= (t - 1) & t; u != t; u= (u - 1) & t) {  // t の真の部分集合 (t = 0 なら何もしない)
    x+= h[u];
    if(!u) break;
   }
   phi(lo + t, x);
   h[t]= x;
  }
  for(int t= 0; t < w; ++t) Y[t]= h[t];
  for(int k= 0; k < b; ++k)
   for(int t= 0; t < w; ++t)
    if(t >> k & 1) Y[t]+= Y[t ^ (1 << k)];
  return;
 }
 const size_t hw= size_t(1) << (b - 1);
 ozeta_rec(Y, lo, b - 1, phi);
 for(size_t t= 0; t < hw; ++t) Y[hw + t]+= Y[t];
 ozeta_rec(Y + hw, lo + hw, b - 1, phi);
 for(size_t t= 0; t < hw; ++t) Y[hw + t]+= Y[t];
}
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
  const size_t N= size_t(1) << n;
  vector<u32> A(N, 0);
  for(int x: s) A[x]= 1;
  for(int b= 0; b < n; ++b)
   for(size_t a= 0; a < N; ++a)
    if(a >> b & 1) A[a]+= A[a ^ (size_t(1) << b)];
  vector<Pair> Y(N);
  Z prev= Z(1);  // dp[0] = 1
  ozeta_rec(Y.data(), 0, n, [&](size_t k, Pair& x) {
   const Z dp= prev, dp2= Z(A[k]) * (x.a + dp) - x.b;
   x= Pair{dp, dp2};
   prev= dp2;
  });
  ans= prev.val();
 }
 u32 answer() const { return ans; }
};
