#pragma once
#include "_shared/modulo-test/_common.hpp"
// plantard に、決まった値 B を掛けるときの B' = B rho mod 2^64 の前計算を足した版 (algo-notes の plantard.md の 8 節)。
// reduce の (a B) rho mod 2^64 を a B' mod 2^64 で置き換えるので、掛け算は a B' と (q + 1) mod の 2 回になる。
// https://zenn.dev/yatyou/articles/f8a3969c6e9f7b
struct MP {  // mod < 2^32/phi, mod は奇数
 u32 mod;
 constexpr MP(): mod(0), r2(0), iv(0) {}
 constexpr MP(u32 m): mod(m), r2(u32(-u128(m) % m)), iv(inv(m)) {}
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return reduce(n); }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u64 fix(u32 w) const { return u64(w) * iv; }
 constexpr inline u32 mul_fix(u32 a, u64 w) const { return (u128((u64(a) * w) | u32(-1)) * mod) >> 64; }
private:
 u32 r2;
 u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
};
