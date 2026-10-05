#pragma once
// 素朴 reference (golden): 素数 p ごとに a^((2^64-1)/p) を 2 乗と掛け算の繰り返しで求め、1 でない p を掛ける。掛け算は
// 4 bit ずつの窓で 128 bit の積を作ってから畳む (pclmul を使わない。self-gf2-64-log-any の checker と同じ)。
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_ref {
inline u64 mul(u64 a, u64 b) {
 u64 tl[16], th[16];
 tl[0]= th[0]= 0, tl[1]= a, th[1]= 0;
 for(int i= 2; i < 16; i+= 2) {
  tl[i]= tl[i / 2] << 1, th[i]= (th[i / 2] << 1) | (tl[i / 2] >> 63);
  tl[i + 1]= tl[i] ^ a, th[i + 1]= th[i];
 }
 u64 lo= 0, hi= 0;
 for(int s= 60; s >= 0; s-= 4) {
  hi= (hi << 4) | (lo >> 60), lo<<= 4;
  const unsigned nib= (b >> s) & 15;
  lo^= tl[nib], hi^= th[nib];
 }
 // hi·x^64 ≡ hi·(x^4 + x^3 + x + 1)。ずらしてはみ出た分 (4 bit 未満) をもう一度同じように畳む。
 const u64 over= (hi >> 63) ^ (hi >> 61) ^ (hi >> 60);
 return lo ^ hi ^ (hi << 1) ^ (hi << 3) ^ (hi << 4) ^ over ^ (over << 1) ^ (over << 3) ^ (over << 4);
}
inline u64 pow(u64 a, u64 e) {
 u64 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
}  // namespace gf2_64_ref
inline vector<u64> run(const vector<u64>& as) {
 constexpr u64 M= ~0ull, PR[7]= {3, 5, 17, 257, 641, 65537, 6700417};
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) {
  u64 o= 1;
  for(u64 p: PR)
   if(gf2_64_ref::pow(as[i], M / p) != 1) o*= p;
  ans[i]= o;
 }
 return ans;
}
