#pragma once
#include "_shared/modulo-test/_common.hpp"
// plantard の、法と前計算の値をクラスの static 変数に置いた版。実行時の法を ZMod<0> のように型の static 変数で持つ形を真似る。
// 配列への書き込みのたびに法を読み直すことになるか、コンパイラが決まった値の前計算をやめないかを、plantard と比べる。
// https://zenn.dev/yatyou/articles/f8a3969c6e9f7b
struct MP {  // mod < 2^32/phi, mod は奇数
 static inline u32 mod;
 MP() {}
 MP(u32 m) { mod= m, r2= u32(-u128(m) % m), iv= inv(m); }
 static inline u32 set(u32 n) { return mul(n, r2); }
 static inline u32 get(u32 n) { return reduce(n); }
 static inline u32 plus(u32 l, u32 r) { return l+= r, l < mod ? l : l - mod; }
 static inline u32 mul(u32 l, u32 r) { return reduce(u64(l) * r); }
 static inline u32 fix(u32 w) { return w; }
 static inline u32 mul_fix(u32 a, u32 w) { return mul(a, w); }
private:
 static inline u32 r2;
 static inline u64 iv;
 static constexpr u64 inv(u64 n, int e= 6, u64 x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 static inline u32 reduce(u64 w) { return (u128((w * iv) | u32(-1)) * mod) >> 64; }
};
