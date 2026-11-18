#pragma once
// 参照実装 (ライブラリを読まない、intrinsics を使わない)。割る式を monic にして筆算で割る。
#include <utility>
#include <vector>
inline pair<vector<u32>, vector<u32>> run(const vector<u32>& a, const vector<u32>& b) {
 constexpr u64 M= 998244353;
 auto pw= [](u64 x, u64 e) {
  u64 r= 1;
  for(; e; e>>= 1, x= x * x % M)
   if(e & 1) r= r * x % M;
  return r;
 };
 const int n= int(a.size()), m= int(b.size());
 const u64 il= pw(b[m - 1], M - 2);
 vector<u64> r(a.begin(), a.end());
 vector<u32> q(n - m + 1);
 for(int i= n - 1; i >= m - 1; --i) {
  const u64 c= r[i] * il % M;
  q[i - m + 1]= u32(c);
  if(!c) continue;
  for(int j= 0; j < m - 1; ++j) r[i - m + 1 + j]= (r[i - m + 1 + j] + (M - c) * b[j]) % M;
 }
 return {q, vector<u32>(r.begin(), r.begin() + (m - 1))};
}
