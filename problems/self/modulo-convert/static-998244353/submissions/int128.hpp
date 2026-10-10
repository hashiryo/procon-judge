#pragma once
#include "_shared/modulo-test/_common.hpp"
// 今の Library の MInt(__int128_t n) と同じ変換。整数を __int128_t で受けて % mod するので、libgcc の __modti3 を呼ぶ。
struct MP {  // mod < 2^31
 u32 mod;
 constexpr MP(u32 m): mod(m) {}
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 from_i64(__int128_t n) const { return u32(n < 0 ? ((n= (-n) % mod) ? mod - n : n) : n % mod); }
};
