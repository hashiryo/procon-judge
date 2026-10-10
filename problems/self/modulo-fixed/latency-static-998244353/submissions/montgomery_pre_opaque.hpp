#pragma once
#include "_shared/modulo-test/_common.hpp"
// montgomery_pre の、前計算した b iv mod 2^32 を asm で隠した版。法が定数だと GCC は b iv を「b と定数 iv の積」と見抜き、
// a (b iv) を (a b) iv に組み替えて鎖の掛け算を 3 回に戻す。b iv を隠してこの組み替えを止める。
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
 inline F fix(u32 w) const {
  u32 wi= w * iv;
  asm("" : "+r"(wi));
  return F{w, wi};
 }
 constexpr inline u32 mul_fix(u32 a, F f) const { return (u64(a) * f.w + u64(u32(a * f.wi)) * mod) >> 32; }
private:
 u32 iv, r2;
 static constexpr u32 inv(u32 n, int e= 5, u32 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u32 reduce(u64 w) const { return (w + (u32(u32(w) * iv) * u64(mod))) >> 32; }
};
