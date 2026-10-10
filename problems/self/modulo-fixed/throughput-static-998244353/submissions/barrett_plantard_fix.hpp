#pragma once
#include "_shared/modulo-test/_common.hpp"
// 値をそのまま持つ barrett に、決まった値を掛ける口として Plantard の前計算を載せた版。fix と mul_fix だけが奇数の法を要る。
// w を Plantard の表現 w~ = -w R^2 mod mod (R = 2^32) に直し、B' = w~ rho mod 2^64 を前計算しておく。Plantard の還元は
// a w~ (-R^-2) = a w を返すので、そのままの値 a (2 mod 未満) に掛けた結果も、そのままの値 (mod 未満) になる。
// 掛け算は a B' と (q + 1) mod の 2 回。B' は asm で隠し、法が定数のときの組み替え (a B' を (a w~) rho に戻す) を止める。
// https://zenn.dev/yatyou/articles/f8a3969c6e9f7b
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(): mod(0), x(0), r4(0), iv(0) {}
 constexpr MP(u32 m): mod(m), x(u64(-1) / m), r4(m & 1 ? u32(-u128(m) % m) : 0), iv(m & 1 ? inv(m) : 0) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return rem(u64(l) * r); }
 inline u64 fix(u32 w) const {
  u64 b= u64(plantard(u64(get(w)) * r4)) * iv;
  asm("" : "+r"(b));
  return b;
 }
 constexpr inline u32 mul_fix(u32 a, u64 b) const { return plantard_pre(a, b); }
private:
 u64 x;
 u32 r4;
 u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 quo(u64 n) const { return (u128(n) * x) >> 64; }
 constexpr inline u32 rem(u64 n) const { return n - u64(quo(n)) * mod; }
 constexpr inline u32 plantard(u64 w) const { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
 constexpr inline u32 plantard_pre(u32 a, u64 b) const { return (u128((u64(a) * b) | u32(-1)) * mod) >> 64; }
};
