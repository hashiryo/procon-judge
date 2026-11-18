#pragma once
// 参照実装 (ライブラリを読まない)。P(X) = Π (X - x_j) を作り、点ごとに P/(X - x_i) を組立除法で求めて P'(x_i) で割って足す。
#include <vector>
inline vector<u32> run(const vector<u32>& x, const vector<u32>& y) {
 constexpr u64 M= 998244353;
 auto pw= [](u64 a, u64 e) {
  u64 r= 1;
  for(a%= M; e; e>>= 1, a= a * a % M)
   if(e & 1) r= r * a % M;
  return r;
 };
 const int n= int(x.size());
 vector<u64> P(n + 1, 0);
 P[0]= 1;
 for(int j= 0; j < n; ++j) {
  for(int k= j + 1; k > 0; --k) P[k]= (P[k - 1] + (M - x[j]) * P[k]) % M;
  P[0]= (M - x[j]) * P[0] % M;
 }
 vector<u64> f(n, 0), q(n);
 for(int i= 0; i < n; ++i) {
  q[n - 1]= P[n];
  for(int k= n - 1; k > 0; --k) q[k - 1]= (P[k] + x[i] * q[k]) % M;
  u64 d= 0;
  for(int k= n - 1; k >= 0; --k) d= (d * x[i] + q[k]) % M;
  const u64 w= y[i] * pw(d, M - 2) % M;
  for(int k= 0; k < n; ++k) f[k]= (f[k] + w * q[k]) % M;
 }
 return vector<u32>(f.begin(), f.end());
}
