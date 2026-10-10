#pragma once
#include "_shared/modulo-test/_common.hpp"
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 constexpr MP(): mod(0), iv(0), r2(0) {}
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m) {}
 constexpr inline int set(u32 n) const { return compress(mul_u(n, r2)); }
 constexpr inline u32 get(int n) const { u32 u= reduce_u(expand(n)); return u >= mod ? u - mod : u; }
 constexpr inline int mul(int l, int r) const { return compress(mul_u(expand(l), expand(r))); }
 constexpr inline int plus(int l, int r) const {
  i64 s= i64(l) + r;
  i64 lim= mod;
  if(s >= lim) s-= lim << 1;
  if(s < -lim) s+= lim << 1;
  return int(s);
 }
 constexpr inline int diff(int l, int r) const { return plus(l, -r); }
private:
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 expand(int x) const { return x < 0 ? u32(x + (i64(mod) << 1)) : u32(x); }
 constexpr inline int compress(u32 x) const { return x >= mod ? int(x - (mod << 1)) : int(x); }
 constexpr inline u32 mul_u(u32 l, u32 r) const { return reduce_u(u64(l) * r); }
 constexpr inline u32 reduce_u(u64 w) const { return u32((w + u64(u32(w) * iv) * mod) >> 32); }
};
