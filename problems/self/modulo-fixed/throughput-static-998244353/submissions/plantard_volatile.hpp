#pragma once
#include "_shared/modulo-test/_common.hpp"
// plantard の、法の定数を asm で隠した版 (modulo-test の plantard_2_volatile と同じ隠し方)。決まった値の前計算は書かない。
// modulo-test では、法がコンパイル時の定数だと GCC が Plantard の積の順を変え、片方が決まった積の前計算をしなくなっていた。
// 隠して実行時の値に見せたときと、隠さない plantard と、前計算を書いた plantard_pre を比べる。
// https://zenn.dev/yatyou/articles/f8a3969c6e9f7b
struct MP {  // mod < 2^32/phi, mod は奇数
 u32 mod;
 MP(): mod(0), r2(0), iv(0) {}
 MP(u32 m): mod(m), r2(u32(-u128(m) % m)), iv(inv(m)) { asm volatile("" : "+r"(mod), "+r"(iv), "+r"(r2)); }
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return reduce(n); }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline u32 fix(u32 w) const { return w; }
 constexpr inline u32 mul_fix(u32 a, u32 w) const { return mul(a, w); }
private:
 u32 r2;
 u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
};
