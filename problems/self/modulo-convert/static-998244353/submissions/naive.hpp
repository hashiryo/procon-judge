#pragma once
#include "_shared/modulo-test/_common.hpp"
// 参照実装。i64 の % で割って、負なら mod を足す。
struct MP {  // mod < 2^31
 u32 mod;
 constexpr MP(u32 m): mod(m) {}
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 from_i64(long long a) const {
  long long r= a % (long long)mod;
  return u32(r < 0 ? r + mod : r);
 }
};
