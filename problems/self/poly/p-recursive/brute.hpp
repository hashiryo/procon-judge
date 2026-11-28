#pragma once
// 愚直解。漸化式をそのまま回す (O(N d e))。crosscheck と小さい N の裏取り用。提出と同じ run の口を持つ。
#include <vector>
#include "_shared/ntt/_reference.hpp"
namespace brute_prec {
using ntt_reference::u32;
using ntt_reference::u64;
constexpr u32 M= 998244353;
inline u32 eval(const std::vector<u32>& f, u64 x) {
 u32 r= 0, xm= u32(x % M);
 for (int i= int(f.size()) - 1; i >= 0; --i) r= u32((u64(r) * xm + f[i]) % M);
 return r;
}
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for (; e; e>>= 1, a= u32(u64(a) * a % M))
  if (e & 1) r= u32(u64(r) * a % M);
 return r;
}
}  // namespace brute_prec
// p[i][j] は p_i の n^j の係数、a0 は a_0..a_{d-1}。p_d(n) != 0 (0 <= n <= N-d) を仮定する。
inline u32 run(const std::vector<std::vector<u32>>& p, const std::vector<u32>& a0, u64 N) {
 using namespace brute_prec;
 const int d= int(p.size()) - 1;
 if (N < u64(d)) return a0[N];
 std::vector<u32> v(a0.begin(), a0.begin() + d), w(d);
 for (u64 n= 0; n + u64(d) <= N; ++n) {
  u32 s= 0;
  for (int i= 0; i < d; ++i) s= u32((s + u64(eval(p[i], n)) * v[i]) % M);
  for (int i= 0; i + 1 < d; ++i) w[i]= v[i + 1];
  w[d - 1]= u32(u64(s) * pw(eval(p[d], n), M - 2) % M);
  w[d - 1]= w[d - 1] ? M - w[d - 1] : 0;
  v.swap(w);
 }
 return v[d - 1];
}
