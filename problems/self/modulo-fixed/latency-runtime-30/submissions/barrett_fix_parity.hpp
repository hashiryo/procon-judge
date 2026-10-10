#pragma once
#include "_shared/modulo-test/_common.hpp"
// barrett_plantard_fix の口を偶数の法にも広げた版。実行時の法 (ZMod<0>) では偶奇が実行時まで分からないので、
// 掛け算のたびに偶奇で分岐し、奇数なら Plantard の前計算 (barrett_plantard_fix と同じ)、偶数なら前計算しない Barrett で掛ける。
// 法は途中で変わらないので分岐の予測は当たり続ける。前計算した B' は asm で隠す。
struct MP {  // mod < 2^30
 u32 mod;
 struct F {
  u64 b;
  u32 w;
 };
 constexpr MP(): mod(0), odd(false), x(0), r4(0), iv(0) {}
 constexpr MP(u32 m): mod(m), odd(m & 1), x(u64(-1) / m), r4(m & 1 ? u32(-u128(m) % m) : 0), iv(m & 1 ? inv(m) : 0) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return rem(u64(l) * r); }
 inline F fix(u32 w) const {
  w= get(w);
  u64 b= odd ? u64(plantard(u64(w) * r4)) * iv : 0;
  asm("" : "+r"(b));
  return F{b, w};
 }
 constexpr inline u32 mul_fix(u32 a, F f) const { return odd ? u32((u128((u64(a) * f.b) | u32(-1)) * mod) >> 64) : rem(u64(a) * f.w); }
private:
 bool odd;
 u64 x;
 u32 r4;
 u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 quo(u64 n) const { return (u128(n) * x) >> 64; }
 constexpr inline u32 rem(u64 n) const { return n - u64(quo(n)) * mod; }
 constexpr inline u32 plantard(u64 w) const { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
};
