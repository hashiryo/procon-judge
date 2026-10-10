#pragma once
#include "_shared/modulo-test/_common.hpp"
// Barrett (前計算は floor((2^64 - 1) / mod)) で u64(a) を割り、負なら 2^64 の分を補正する。値は 2 mod 未満で持つ。
// u64 の全範囲の n では、q の誤差が 2 まであって n - q mod は 3 mod 未満になるので、2 mod 以上なら 2 mod を引く。
// a < 0 なら u64(a) = a + 2^64 なので、2 mod - (2^64 mod mod) を足し、もう一度 2 mod 以上なら引く。分岐は cmov になる形で書く。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(u32 m): mod(m), x(u64(-1) / m), k(2 * m - u32((u64(-1) % m + 1) % m)) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 from_i64(long long a) const {
  u64 n= u64(a);
  u32 r= u32(n - u64((u128(n) * x) >> 64) * mod);
  r= r >= (mod << 1) ? r - (mod << 1) : r;
  r+= u32(a >> 63) & k;
  return r >= (mod << 1) ? r - (mod << 1) : r;
 }
private:
 u64 x;
 u32 k;
};
