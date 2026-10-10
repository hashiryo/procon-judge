#pragma once
#include "_shared/modulo-test/_common.hpp"
// mod = 119 * 2^23 + 1 固定。Montgomery の q * mod を q + (q * 119) << 23 に置き換える。
struct MP {
 static constexpr u32 mod= 998244353;
 static constexpr u32 iv= 998244351;
 static constexpr u32 r2= 932051910;
 constexpr MP()= default;
 constexpr MP(u32) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u32 norm(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 diff(u32 l, u32 r) const { return l-= r, l >> 31 ? l + (mod << 1) : l; }
private:
 static constexpr inline u32 reduce(u64 w) {
  u32 t= u32(w) * iv;
  return u32((w + t + ((u64(t) * 119) << 23)) >> 32);
 }
};
