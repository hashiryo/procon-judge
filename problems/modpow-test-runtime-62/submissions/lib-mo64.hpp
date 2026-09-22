#pragma once
// Library の MP_Mo64 (64 bit の Montgomery。奇数 mod < 2^62) と math_internal::pow (二分累乗) をそのまま使う。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
struct MP {
 math_internal::MP_Mo64 md;
 constexpr MP(u64 m): md(m) {}
 constexpr inline u64 set(u64 n) const { return md.set(n); }
 constexpr inline u64 get(u64 n) const { return md.get(n); }
 constexpr inline u64 pow(u64 a, u64 b) const { return math_internal::pow(a, b, md); }
};
