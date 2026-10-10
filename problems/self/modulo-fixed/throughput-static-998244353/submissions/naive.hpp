#pragma once
#include "_shared/modulo-test/_common.hpp"
// 参照実装。毎回 u64 の % で還元する。決まった値の前計算はしない (fix はそのまま返す)。
struct MP {  // mod < 2^32
 u32 mod;
 constexpr MP(): mod(0) {}
 constexpr MP(u32 m): mod(m) {}
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 mul(u32 l, u32 r) const { return u64(l) * r % mod; }
 constexpr inline u32 fix(u32 w) const { return w; }
 constexpr inline u32 mul_fix(u32 a, u32 w) const { return mul(a, w); }
};
