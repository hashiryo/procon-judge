#pragma once
#include "_shared/modulo-test/_common.hpp"
// modulo-test の barrett_reduction_K=64_floor。法の偶奇を問わない。決まった値の前計算はしない (fix はそのまま返す)。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(): mod(0), x(0) {}
 constexpr MP(u32 m): mod(m), x(-u64(m) / m + 1) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return rem(u64(l) * r); }
 constexpr inline u32 fix(u32 w) const { return w; }
 constexpr inline u32 mul_fix(u32 a, u32 w) const { return mul(a, w); }
private:
 u64 x;
 constexpr inline u32 quo(u64 n) const { return (u128(n) * x) >> 64; }
 constexpr inline u32 rem(u64 n) const { return n - u64(quo(n)) * mod; }
};
