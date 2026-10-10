#pragma once
#include "_shared/modulo-test/_common.hpp"
// barrett_plantard_fix の、法を u64 の static 変数で持つ版。u32 で持つと配列への書き込みのたびに法を読み直すので、型を u64 にして別名にならないようにする。
// 実行時の法を ZMod<0> のように型の static 変数で持つ形を真似る。前計算した B' はハーネスの局所変数に入る。
struct MP {  // mod < 2^30
 MP() {}
 MP(u32 m) { mod_= m, x= u64(-1) / m, r4= m & 1 ? u32(-u128(m) % m) : 0, iv= m & 1 ? inv(m) : 0; }
 static inline u32 set(u32 n) { return n; }
 static inline u32 get(u32 n) { return n >= mod() ? n - mod() : n; }
 static inline u32 plus(u32 l, u32 r) { return l+= r, l < (mod() << 1) ? l : l - (mod() << 1); }
 static inline u32 mul(u32 l, u32 r) { return rem(u64(l) * r); }
 static inline u64 fix(u32 w) {
  u64 b= u64(plantard(u64(get(w)) * r4)) * iv;
  asm("" : "+r"(b));
  return b;
 }
 static inline u32 mul_fix(u32 a, u64 b) { return (u128((u64(a) * b) | u32(-1)) * mod()) >> 64; }
private:
 static inline u64 mod_;
 static inline u64 x, r4, iv;
 static inline u32 mod() { return u32(mod_); }
 static constexpr u64 inv(u64 n, int e= 6, u64 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 static inline u32 quo(u64 n) { return (u128(n) * x) >> 64; }
 static inline u32 rem(u64 n) { return n - u64(quo(n)) * mod(); }
 static inline u32 plantard(u64 w) { return (u128((w * iv) | u32(-1)) * mod()) >> 64; }
};
