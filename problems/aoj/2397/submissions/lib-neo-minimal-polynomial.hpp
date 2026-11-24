#pragma once
#include <random>
#include "pj.hpp"
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/berlekamp_massey.hpp"
#include "neo/base/core/convolve/naive.hpp"
#include "neo/base/core/poly/poly_divmod.hpp"
// 行列とベクトルの組 (A, b) の最小多項式を、ランダムな u の数列 u^T A^i b に berlekamp_massey をかけて求める。A^k b は
// x^k を最小多項式で割った余り (convolve_naive の 2 乗と poly_divmod の剰余で O(W^2 log k)) の係数で、クリロフ列を足し合わせる。
struct Solver {
 using Z= ZMod<int(1e9 + 9)>;
 int w;
 i64 h;
 vector<array<i64, 2>> obs;
 Z ans;
 Solver(int w, i64 h, const vector<array<i64, 2>>& obs): w(w), h(h), obs(obs) {}
 vector<Z> mul_a(const vector<Z>& v) const {
  vector<Z> r(w);
  for(int i= 0; i < w; ++i) {
   r[i]= v[i];
   if(i) r[i]+= v[i - 1];
   if(i + 1 < w) r[i]+= v[i + 1];
  }
  return r;
 }
 vector<Z> pow_a(vector<Z> b, i64 k) const {
  static mt19937_64 rng(12345);
  vector<Z> u(w);
  for(auto& x: u) x= Z(rng() % (Z::mod() - 1) + 1);
  vector<vector<Z>> bs;
  vector<Z> s;
  for(int i= 0; i < 2 * w + 2; ++i) {
   if(i <= w) bs.push_back(b);
   Z t= 0;
   for(int j= 0; j < w; ++j) t+= u[j] * b[j];
   s.push_back(t);
   b= mul_a(b);
  }
  const auto c= berlekamp_massey(s);
  const int d= c.size();
  if(d == 0) return vector<Z>(w);  // b = 0
  // P(x) = x^d - c_1 x^{d-1} - ... - c_d
  vector<Z> P(d + 1);
  P[d]= 1;
  for(int j= 1; j <= d; ++j) P[d - j]= -c[j - 1];
  vector<Z> r= {1};  // x^k mod P
  for(int bit= 62; bit >= 0; --bit) {
   if(!r.empty()) r= poly_divmod(convolve_naive(r, r), P).second;
   if(k >> bit & 1) {
    r.insert(r.begin(), Z(0));
    r= poly_divmod(r, P).second;
   }
  }
  vector<Z> ret(w);
  for(int i= 0; i < (int)r.size(); ++i)
   for(int j= 0; j < w; ++j) ret[j]+= r[i] * bs[i][j];
  return ret;
 }
 void run() {
  vector<Z> b(w);
  b[0]= 1;
  i64 y= 0;
  const int n= (int)obs.size();
  for(int i= 0; i < n; ++i) {
   b= pow_a(b, obs[i][0] - y);
   int j= i;
   while(j < n && obs[i][0] == obs[j][0]) b[obs[j++][1]]= 0;
   i= j - 1;
   y= obs[i][0];
  }
  b= pow_a(b, h - 1 - y);
  ans= b[w - 1];
 }
 i64 answer() const { return ans.val(); }
};
