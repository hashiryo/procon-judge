#pragma once
// Library の MP_Mo32 (32 bit の Montgomery。奇数 mod < 2^30 (ModInt が使う範囲)) と math_internal::pow (二分累乗) をそのまま使う。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
struct MP {
 math_internal::MP_Mo32 md;
 constexpr MP(u32 m): md(m) {}
 constexpr inline u32 set(u32 n) const { return md.set(n); }
 constexpr inline u32 get(u32 n) const { return md.get(n); }
 constexpr inline u32 pow(u32 a, u64 b) const { return math_internal::pow(a, b, md); }
};
