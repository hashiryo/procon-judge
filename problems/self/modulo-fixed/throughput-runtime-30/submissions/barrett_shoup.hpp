#pragma once
#include "_shared/modulo-test/_common.hpp"
// barrett に、決まった値 w を掛ける Shoup の方法 (NTL の MulModPrecon) を足した版。法の偶奇を問わない。
// w ごとに w' = floor(w 2^32 / mod) を前計算しておき、a < 2^32 に対して q = floor(a w' / 2^32)、r = a w - q mod とすると、
// 0 <= r < 2 mod になる。r が 2^32 未満なので、a w と q mod は下位 32 bit だけ計算すればよい。
// a w と a w' は互いに独立なので、依存の鎖に乗る掛け算は a w' と q mod の 2 回。
struct MP {  // mod < 2^30
 u32 mod;
 struct F {
  u32 w, wq;
 };
 constexpr MP(): mod(0), x(0) {}
 constexpr MP(u32 m): mod(m), x(-u64(m) / m + 1) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return rem(u64(l) * r); }
 constexpr inline F fix(u32 w) const { return w= get(w), F{w, u32((u64(w) << 32) / mod)}; }
 constexpr inline u32 mul_fix(u32 a, F f) const { return a * f.w - u32((u64(a) * f.wq) >> 32) * mod; }
private:
 u64 x;
 constexpr inline u32 quo(u64 n) const { return (u128(n) * x) >> 64; }
 constexpr inline u32 rem(u64 n) const { return n - u64(quo(n)) * mod; }
};
