#pragma once
// 参照実装 (ライブラリを読まない、intrinsics を使わない)。f(0) ≠ 0 を前提に、g = f^m を g' f = m f' g から出る
// i f_0 g_i = Σ_j ((m + 1) j - i) f_j g_{i-j} で、項ごとに剰余を取って回す。1/i は inv[i] = -(M / i) inv[M mod i] で作る。
#include <utility>
#include <vector>
inline vector<u32> run(int n, u64 m, const vector<pair<int, u32>>& f) {
 constexpr u64 M= 998244353;
 auto pw= [](u64 x, u64 e) {
  u64 r= 1;
  for(; e; e>>= 1, x= x * x % M)
   if(e & 1) r= r * x % M;
  return r;
 };
 vector<u32> g(n, 0);
 if(n == 0) return g;
 const u64 f0= f[0].second, if0= pw(f0, M - 2), m1= (m % M + 1) % M;
 vector<u64> inv(n + 1, 1);
 for(int i= 2; i < n; ++i) inv[i]= (M - M / i) * inv[M % i] % M;
 g[0]= u32(pw(f0, m));
 for(int i= 1; i < n; ++i) {
  u64 s= 0;
  for(size_t k= 1; k < f.size() && f[k].first <= i; ++k) {
   const u64 j= f[k].first, c= (m1 * j % M + M - u64(i) % M) % M;
   s= (s + c * f[k].second % M * g[i - j]) % M;
  }
  g[i]= u32(s * inv[i] % M * if0 % M);
 }
 return g;
}
