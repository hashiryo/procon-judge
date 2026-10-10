#pragma once
#include "_shared/modulo-test/_common.hpp"
// 参照実装。積を 1 つずつ u64 の % で割って足す。
struct MP {  // mod < 2^31
 u32 mod;
 constexpr MP(u32 m): mod(m) {}
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 inline u32 dot(const u32* a, const u32* b, size_t n) const {
  u64 s= 0;
  for (size_t k= 0; k < n; ++k) s= (s + u64(a[k]) * b[k]) % mod;
  return u32(s);
 }
};
