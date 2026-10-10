#pragma once
#include "_shared/modulo-test/_common.hpp"
// modulo-test の montgomery_plus で、積を 1 つずつ還元して足す。Montgomery の還元は入力が mod 2^32 未満でないといけないので、
// 2 mod 未満の値どうしの積を 2 つ以上足してから還元することはできない (mod が 2^30 に近いとき)。
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m) {}
 constexpr inline u32 set(u32 n) const { return reduce(u64(n) * r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 inline u32 dot(const u32* a, const u32* b, size_t n) const {
  u32 s= 0;
  for (size_t k= 0; k < n; ++k) s= plus(s, mul(a[k], b[k]));
  return s;
 }
private:
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 reduce(u64 w) const { return (w + (u32(u32(w) * iv) * u64(mod))) >> 32; }
};
