#pragma once
// 素朴な実装 (intrinsics を使わない)。u = f / f(0) の u^m を v_i = (Σ_j al_j v_{i-j}) / i - Σ_j u_j v_{i-j}
// (al_j = (m + 1) j u_j) で求め、最後に f(0)^m を掛ける。項の積は u64 に 15 個までためてから Barrett で剰余を取る。
// 1/i は inv[i] = -(M / i) inv[M mod i] で作る。
#include <utility>
#include <vector>
namespace sp_naive {
constexpr u32 M= 998244353;
constexpr u64 BX= ~u64(0) / M;
inline u32 red(u64 t) {
 u64 r= t - u64((__uint128_t(t) * BX) >> 64) * M;
 r= r >= 2 * u64(M) ? r - 2 * u64(M) : r;
 return u32(r >= M ? r - M : r);
}
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= u32(u64(a) * a % M))
  if(e & 1) r= u32(u64(r) * a % M);
 return r;
}
}  // namespace sp_naive
inline vector<u32> run(int n, u64 m, const vector<pair<int, u32>>& f) {
 using namespace sp_naive;
 vector<u32> v(n, 0);
 if(n == 0) return v;
 const u32 f0= f[0].second, if0= pw(f0, M - 2), m1= u32((m % M + 1) % M);
 vector<int> d;
 vector<u32> c, al;
 for(size_t k= 1; k < f.size() && f[k].first < n; ++k) {
  const u32 uk= u32(u64(f[k].second) * if0 % M);
  d.push_back(f[k].first), c.push_back(uk), al.push_back(u32(u64(uk) * m1 % M * f[k].first % M));
 }
 vector<u32> iv(n, 0);
 if(n > 1) iv[1]= 1;
 for(int i= 2; i < n; ++i) iv[i]= u32(u64(M - M / i) * iv[M % i] % M);
 v[0]= 1;
 const int t= int(d.size());
 for(int i= 1; i < n; ++i) {
  u64 sa= 0, sb= 0;
  for(int k= 0, cnt= 0; k < t && d[k] <= i; ++k) {
   sa+= u64(al[k]) * v[i - d[k]], sb+= u64(c[k]) * v[i - d[k]];
   if(++cnt == 15) sa= red(sa), sb= red(sb), cnt= 0;
  }
  const u32 x= red(u64(red(sa)) * iv[i]), b= red(sb);
  v[i]= x >= b ? x - b : x + M - b;
 }
 const u32 a= pw(f0, m);
 if(a != 1)
  for(auto& x: v) x= u32(u64(x) * a % M);
 return v;
}
