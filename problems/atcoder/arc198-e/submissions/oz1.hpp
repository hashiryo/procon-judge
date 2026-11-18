#pragma once
// 書き比べ: 公式解説の、添字の小さい順に値を足しながら部分集合の和を問う構造 (オンラインのゼータ) で解く。
// 構造は、足した添字 [0, size) を size の 2 進の 1 の bit ごとの揃った区間 (区画) に分け、区画ごとに中の値のゼータ変換を持つ。
//   sum(x) = sum_{j < size, j ⊆ x} x_j: 区画 [c, c + 2^b) は [c ⊆ x] y[c + (x mod 2^b)] を足す。O(n)。
//   push(v): 大きさ 1 の区画を足し、末尾の 2 つの区画の大きさが等しい間、上の区画に下の区画の値を足して 1 つにする。均して O(n)。
// dp[k] = dp2[k - 1] (dp[0] = 1)、D(k) = sum_{j ⊆ k} dp[j]、dp2[k] = A(k) D(k) - sum_{j ⊊ k} dp2[j]。答えは dp2[2^N - 1]。
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
template <class T> struct OnlineSubsetSum {
 int n;
 vector<T> y;
 size_t sz= 0;
 explicit OnlineSubsetSum(int n_): n(n_), y(size_t(1) << n_) {}
 T sum(size_t x) const {
  T r= T();
  size_t c= 0;
  for(int b= n + 1; b-- > 0;)  // size = 2^n のときは全体が 1 つの区画
   if(sz >> b & 1) {
    if(!(c & ~x)) r+= y[c | (x & ((size_t(1) << b) - 1))];
    c|= size_t(1) << b;
   }
  return r;
 }
 void push(T v) {
  y[sz++]= v;
  for(int b= 0; b < n && !(sz >> b & 1); ++b) {
   const size_t w= size_t(1) << b;
   T* H= y.data() + sz - w;
   const T* L= H - w;
   for(size_t t= 0; t < w; ++t) H[t]+= L[t];
  }
 }
};
struct Solver {
 using Z= ZMod<998244353>;
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
  OnlineSubsetSum<Z> D(n), S2(n);
  Z prev= Z(1);  // dp[0] = 1
  for(size_t k= 0; k < N; ++k) {
   const Z dp= prev;
   D.push(dp);
   const Z dp2= Z(A[k]) * D.sum(k) - S2.sum(k);
   S2.push(dp2);
   prev= dp2;
  }
  ans= prev.val();
 }
 u32 answer() const { return ans; }
};
