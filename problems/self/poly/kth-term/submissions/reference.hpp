#pragma once
// 参照実装 (ライブラリを読まない)。Bostan–Mori の方法で、Q = 1 - Σ c_j x^j、P = A Q mod x^d として [x^k] P/Q を求める。
// Q = E(x^2) + x O(x^2)、P = Pe(x^2) + x Po(x^2) と分けると、Q(x) Q(-x) = E(y)^2 - y O(y)^2、
// P(x) Q(-x) の偶数部 = Pe E - y Po O、奇数部 = Po E - Pe O (y = x^2) なので、半分の大きさの積 4 回で 1 段進む。
#include <vector>
namespace ref_kth {
constexpr u32 M= 998244353;
// a * b (長さ n + m - 1)。行ごとに u64 にため、16 行ごとに剰余を取る。
inline vector<u32> mul(const vector<u32>& a, const vector<u32>& b) {
 if(a.empty() || b.empty()) return {};
 const int n= int(a.size()), m= int(b.size());
 vector<u64> acc(n + m - 1);
 for(int i= 0; i < n; ++i) {
  const u64 x= a[i];
  for(int j= 0; j < m; ++j) acc[i + j]+= x * b[j];
  if((i & 15) == 15)
   for(auto& v: acc) v%= M;
 }
 vector<u32> c(n + m - 1);
 for(int k= 0; k < n + m - 1; ++k) c[k]= u32(acc[k] % M);
 return c;
}
}  // namespace ref_kth
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 using namespace ref_kth;
 const int d= int(a.size());
 if(k < u64(d)) return a[k];
 vector<u32> Q(d + 1);
 Q[0]= 1;
 for(int j= 1; j <= d; ++j) Q[j]= c[j - 1] ? M - c[j - 1] : 0;
 vector<u32> P= mul(a, Q);
 P.resize(d);
 for(; k; k>>= 1) {
  vector<u32> E, O, Pe, Po;
  for(int i= 0; i <= d; ++i) (i & 1 ? O : E).push_back(Q[i]);
  for(int i= 0; i < d; ++i) (i & 1 ? Po : Pe).push_back(P[i]);
  const vector<u32> EE= mul(E, E), OO= mul(O, O);
  vector<u32> nQ(d + 1);
  for(int i= 0; i <= d; ++i) {
   const u32 x= i < int(EE.size()) ? EE[i] : 0, y= i >= 1 && i - 1 < int(OO.size()) ? OO[i - 1] : 0;
   nQ[i]= x >= y ? x - y : x + M - y;
  }
  vector<u32> nP(d);
  if(k & 1) {
   const vector<u32> A1= mul(Po, E), A2= mul(Pe, O);
   for(int i= 0; i < d; ++i) {
    const u32 x= i < int(A1.size()) ? A1[i] : 0, y= i < int(A2.size()) ? A2[i] : 0;
    nP[i]= x >= y ? x - y : x + M - y;
   }
  } else {
   const vector<u32> A1= mul(Pe, E), A2= mul(Po, O);
   for(int i= 0; i < d; ++i) {
    const u32 x= i < int(A1.size()) ? A1[i] : 0, y= i >= 1 && i - 1 < int(A2.size()) ? A2[i - 1] : 0;
    nP[i]= x >= y ? x - y : x + M - y;
   }
  }
  P.swap(nP), Q.swap(nQ);
 }
 return P[0];
}
