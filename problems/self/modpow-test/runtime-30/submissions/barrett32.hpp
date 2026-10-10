#pragma once
#include "_shared/modulo-test/_common.hpp"
// modulo-test の barrett_reduction_K=64_floor の還元に、naive32 と同じ二分累乗を付けた版。法の偶奇を問わない。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(): mod(0), x(0) {}
 constexpr MP(u32 m): mod(m), x(-u64(m) / m + 1) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return rem(u64(l) * r); }
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
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
 u64 x;
 constexpr inline u32 quo(u64 n) const { return (u128(n) * x) >> 64; }
 constexpr inline u32 rem(u64 n) const { return n - u64(quo(n)) * mod; }
};
