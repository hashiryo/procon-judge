#pragma once
#include "_shared/modulo-test/_common.hpp"
// Montgomery 32 の加算形リダクション。減算形 (montgomery.hpp) より GCC の自動ベクトル化に乗りやすい形。
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 constexpr MP(): mod(0), iv(0), r2(0) {}
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u32 norm(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 diff(u32 l, u32 r) const { return l-= r, l >> 31 ? l + (mod << 1) : l; }
private:
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return u32((w + u64(u32(w) * iv) * mod) >> 32); }
};
