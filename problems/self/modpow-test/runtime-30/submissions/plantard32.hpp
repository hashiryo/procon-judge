#pragma once
#include "_shared/modulo-test/_common.hpp"
// plantard32_fermat から、指数を p-1 で割った余りに縮める前処理を抜いた版。
// 二分累乗のループは naive32 と同じにして、還元の方式だけを比べる。
struct MP {  // mod < 2^32/phi, mod は奇数
 u32 mod;
 constexpr MP(): mod(0), r2(0), iv(0) {}
 constexpr MP(u32 m): mod(m), r2(u32(-u128(m) % m)), iv(inv(m)) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return reduce(n); }
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
 u32 r2;
 u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
};
