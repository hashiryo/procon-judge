#pragma once
// 参照実装 (ライブラリを読まない、intrinsics を使わない)。f(x) = Σ_k y_k Π_{j≠k} (x-j)/(k-j) を、前後からの (x - j) の累積積と
// 階乗の逆元で O(K) で求める。x mod M が 0..K-1 のどれかなら y をそのまま返す。
#include <vector>
inline u32 run(const vector<u32>& y, u64 x0) {
 constexpr u64 M= 998244353;
 const int K= int(y.size());
 const u64 x= x0 % M;
 if(x < u64(K)) return y[x];
 auto pw= [](u64 a, u64 e) {
  u64 r= 1;
  for(; e; e>>= 1, a= a * a % M)
   if(e & 1) r= r * a % M;
  return r;
 };
 vector<u64> suf(K + 1), ifact(K);
 suf[K]= 1;
 for(int j= K - 1; j >= 0; --j) suf[j]= suf[j + 1] * ((x + M - j) % M) % M;
 u64 f= 1;
 for(int j= 1; j < K; ++j) f= f * j % M;
 ifact[K - 1]= pw(f, M - 2);
 for(int j= K - 1; j > 0; --j) ifact[j - 1]= ifact[j] * j % M;
 u64 pre= 1, res= 0;
 for(int k= 0; k < K; ++k) {
  const u64 w= pre * suf[k + 1] % M * ifact[k] % M * ifact[K - 1 - k] % M * y[k] % M;
  res= (K - 1 - k) & 1 ? (res + M - w) % M : (res + w) % M;
  pre= pre * ((x + M - k) % M) % M;
 }
 return u32(res);
}
