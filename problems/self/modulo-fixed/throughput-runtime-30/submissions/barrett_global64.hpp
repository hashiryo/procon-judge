#pragma once
#include "_shared/modulo-test/_common.hpp"
// barrett_global の、法を u64 の static 変数で持つ版。u32 で持つと、配列への u32 の書き込みが法を書き換えうると GCC が見て、
// 1 周ごとに法を読み直す (barrett_global の機械語で確かめた)。型を u64 にして、別名にならないようにする。
struct MP {  // mod < 2^30
 MP() {}
 MP(u32 m) { mod64= m, x= u64(-1) / m; }
 static inline u32 set(u32 n) { return n; }
 static inline u32 get(u32 n) { return n >= mod() ? n - mod() : n; }
 static inline u32 plus(u32 l, u32 r) { return l+= r, l < (mod() << 1) ? l : l - (mod() << 1); }
 static inline u32 mul(u32 l, u32 r) { return rem(u64(l) * r); }
 static inline u32 fix(u32 w) { return w; }
 static inline u32 mul_fix(u32 a, u32 w) { return mul(a, w); }
private:
 static inline u64 mod64, x;
 static inline u32 mod() { return u32(mod64); }
 static inline u32 quo(u64 n) { return (u128(n) * x) >> 64; }
 static inline u32 rem(u64 n) { return n - u64(quo(n)) * mod(); }
};
