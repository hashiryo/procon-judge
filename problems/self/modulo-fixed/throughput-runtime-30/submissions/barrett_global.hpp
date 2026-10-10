#pragma once
#include "_shared/modulo-test/_common.hpp"
// barrett の、法と前計算の値をクラスの static 変数に置いた版。実行時の法を ZMod<0> のように型の static 変数で持つ形を真似る。
// 配列への書き込みのたびに法を読み直すことになるかを、法をオブジェクトに持つ barrett と比べる。決まった値の前計算はしない。
struct MP {  // mod < 2^30
 static inline u32 mod;
 MP() {}
 MP(u32 m) { mod= m, x= -u64(m) / m + 1; }
 static inline u32 set(u32 n) { return n; }
 static inline u32 get(u32 n) { return n >= mod ? n - mod : n; }
 static inline u32 plus(u32 l, u32 r) { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 static inline u32 mul(u32 l, u32 r) { return rem(u64(l) * r); }
 static inline u32 fix(u32 w) { return w; }
 static inline u32 mul_fix(u32 a, u32 w) { return mul(a, w); }
private:
 static inline u64 x;
 static inline u32 quo(u64 n) { return (u128(n) * x) >> 64; }
 static inline u32 rem(u64 n) { return n - u64(quo(n)) * mod; }
};
