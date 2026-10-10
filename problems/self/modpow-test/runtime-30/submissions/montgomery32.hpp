#pragma once
#include "_shared/modulo-test/_common.hpp"
// modulo-test の montgomery_plus の還元に、naive32 と同じ二分累乗を付けた版。
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 constexpr MP(): mod(0), iv(0), r2(0) {}
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
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
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (w + (u32(u32(w) * iv) * u64(mod))) >> 32; }
};
