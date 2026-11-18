#pragma once
// 参照実装 (ライブラリを読まない、intrinsics を使わない)。教科書どおりの Berlekamp–Massey。
#include <vector>
inline vector<u32> run(const vector<u32>& a) {
 constexpr u64 M= 998244353;
 auto pw= [](u64 x, u64 e) {
  u64 r= 1;
  for(; e; e>>= 1, x= x * x % M)
   if(e & 1) r= r * x % M;
  return r;
 };
 const int n= int(a.size());
 vector<u64> C{1}, B{1};
 int L= 0, m= 1;
 u64 b= 1;
 for(int i= 0; i < n; ++i) {
  u64 d= 0;
  for(int j= 0; j <= L && j < int(C.size()); ++j) d= (d + C[j] * a[i - j]) % M;
  if(d == 0) {
   ++m;
   continue;
  }
  const u64 x= d * pw(b, M - 2) % M;
  vector<u64> T= C;
  if(C.size() < B.size() + m) C.resize(B.size() + m, 0);
  for(size_t j= 0; j < B.size(); ++j) C[j + m]= (C[j + m] + M - x * B[j] % M) % M;
  if(2 * L <= i) L= i + 1 - L, B= T, b= d, m= 1;
  else ++m;
 }
 C.resize(L + 1, 0);
 vector<u32> res(L);
 for(int j= 1; j <= L; ++j) res[j - 1]= u32(C[j] ? M - C[j] : 0);
 return res;
}
