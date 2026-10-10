#pragma once
#include "_shared/modulo-test/_common.hpp"
// 法の偶奇で還元の方式を切り替える。奇数は Plantard (plantard_2 と同じ式)、偶数は Barrett (barrett_reduction_K=64_floor と同じ式)。
// 法は途中で変わらないので分岐の予測は当たり続け、比較は依存の鎖に乗らない。
// 値の持ち方は偶奇で変わる。奇数は Plantard の表現で [0, mod)、偶数はそのままの値で [0, 2 mod)。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(): mod(0), lim(0), odd(false), r2(0), x(0) {}
 constexpr MP(u32 m): mod(m), lim(m & 1 ? m : m << 1), odd(m & 1), r2(m & 1 ? u32(-u128(m) % m) : 0), x(m & 1 ? inv(m) : -u64(m) / m + 1) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return odd ? plantard(u64(l) * r) : barrett(u64(l) * r); }
 constexpr inline u32 set(u32 n) const { return odd ? plantard(u64(n) * r2) : n; }
 constexpr inline u32 get(u32 n) const { return odd ? plantard(n) : n >= mod ? n - mod : n; }
 constexpr inline u32 norm(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < lim ? l : l - lim; }
 constexpr inline u32 diff(u32 l, u32 r) const { return l-= r, l >> 31 ? l + lim : l; }
private:
 u32 lim;
 bool odd;
 u32 r2;
 u64 x;  // 奇数は mod の 2^64 での逆元、偶数は floor(2^64 / mod)
 static constexpr u64 inv(u64 n, int e= 6, u64 y= 1) { return e ? inv(n, e - 1, y * (2 - y * n)) : y; }
 constexpr inline u32 plantard(u64 w) const { return (u128((w * x) | u32(-1)) * mod) >> 64; }
 constexpr inline u32 barrett(u64 n) const { return n - u64(u32((u128(n) * x) >> 64)) * mod; }
};
