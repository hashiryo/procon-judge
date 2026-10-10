#pragma once
#include "_shared/modulo-test/_common.hpp"
// montgomery に、決まった値 b を掛けるときの b iv mod 2^32 の前計算を足した版。
// reduce の (a b mod 2^32) iv を a (b iv) mod 2^32 で置き換えるので、a b と a (b iv) を並べて計算でき、
// 依存の鎖に乗る掛け算は 2 回になる (格子暗号の AVX2 の NTT で回転因子に使われる形)。
struct MP {  // mod < 2^30, mod は奇数
 u32 mod;
 struct F {
  u32 w, wi;
 };
 constexpr MP(): mod(0), iv(0), r2(0) {}
 constexpr MP(u32 m): mod(m), iv(-inv(m)), r2(-u64(m) % m) {}
 constexpr inline u32 set(u32 n) const { return mul(n, r2); }
 constexpr inline u32 get(u32 n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return reduce(u64(l) * r); }
 constexpr inline F fix(u32 w) const { return F{w, w * iv}; }
 constexpr inline u32 mul_fix(u32 a, F f) const { return (u64(a) * f.w + u64(u32(a * f.wi)) * mod) >> 32; }
private:
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (w + (u32(u32(w) * iv) * u64(mod))) >> 32; }
};
