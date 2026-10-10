#pragma once
#include "_shared/modulo-test/_common.hpp"
// modulo-test の parity_branch に、naive32 と同じ二分累乗を付けた版。
// 法の偶奇で還元の方式を切り替える。奇数は Plantard (plantard32 と同じ式)、偶数は Barrett (barrett32 と同じ式)。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(): mod(0), lim(0), odd(false), r2(0), x(0) {}
 constexpr MP(u32 m): mod(m), lim(m & 1 ? m : m << 1), odd(m & 1), r2(m & 1 ? u32(-u128(m) % m) : 0), x(m & 1 ? inv(m) : -u64(m) / m + 1) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return odd ? plantard(u64(l) * r) : barrett(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return odd ? plantard(u64(n) * r2) : n; }
 constexpr inline u32 get(u32 n) const { return odd ? plantard(n) : n >= mod ? n - mod : n; }
 inline u32 pow(u32 base, u32 e) const {
  u32 r= set(1);
  while(e) {
   if(e & 1) r= mul(r, base);
   base= mul(base, base);
   e>>= 1;
  }
  return r;
 }
private:
 u32 lim;
 bool odd;
 u32 r2;
 u64 x;  // 奇数は mod の 2^64 での逆元、偶数は floor(2^64 / mod)
 static constexpr u64 inv(u64 n, int e= 6, u64 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 plantard(u64 w) const { return (u128((w * x) | u32(-1)) * mod) >> 64; }
 constexpr inline u32 barrett(u64 n) const { return n - u64(u32((u128(n) * x) >> 64)) * mod; }
};
