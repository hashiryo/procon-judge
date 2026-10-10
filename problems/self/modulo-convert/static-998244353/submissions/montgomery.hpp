#pragma once
#include "_shared/modulo-test/_common.hpp"
// montgomery_plus の表現 (aR mod mod、2 mod 未満) に、barrett と同じ Barrett の剰余で直してから set で入れる版。
// Montgomery の表現を持つ型は、整数からの変換に Barrett の前計算と掛け算 1 回 (set) が余分に要る。
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m), x(u64(-1) / m), k(2 * m - u32((u64(-1) % m + 1) % m)) {}
 constexpr inline u32 set(u32 n) const { return reduce(u64(n) * r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 from_i64(long long a) const {
  u64 n= u64(a);
  u32 r= u32(n - u64((u128(n) * x) >> 64) * mod);
  r= r >= (mod << 1) ? r - (mod << 1) : r;
  r+= u32(a >> 63) & k;
  return set(r >= (mod << 1) ? r - (mod << 1) : r);
 }
private:
 u32 iv, r2;
 u64 x;
 u32 k;
 static constexpr u32 inv(u32 n, int e= 5, u32 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 reduce(u64 w) const { return (w + (u32(u32(w) * iv) * u64(mod))) >> 32; }
};
