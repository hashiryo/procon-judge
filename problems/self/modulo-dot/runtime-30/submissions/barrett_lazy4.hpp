#pragma once
#include "_shared/modulo-test/_common.hpp"
// Barrett (値は 2 mod 未満) で、積を 4 つ u64 に足してから 1 回だけ還元する。積は 4 mod^2 < 2^62 なので 4 つまで u64 に入る。
struct MP {  // mod < 2^30
 u32 mod;
 constexpr MP(u32 m): mod(m), x(u64(-1) / m) {}
 static constexpr inline u32 set(u32 n) { return n; }
 constexpr inline u32 get(u32 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u32 plus(u32 l, u32 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u32 mul(u32 l, u32 r) const { return rem62(u64(l) * r); }
 inline u32 dot(const u32* a, const u32* b, size_t n) const {
  u32 s= 0;
  size_t k= 0;
  for (; k + 4 <= n; k+= 4) s= plus(s, rem64(u64(a[k]) * b[k] + u64(a[k + 1]) * b[k + 1] + u64(a[k + 2]) * b[k + 2] + u64(a[k + 3]) * b[k + 3]));
  u64 t= 0;
  for (; k < n; ++k) t+= u64(a[k]) * b[k];
  return plus(s, rem64(t));
 }
private:
 u64 x;
 // 2^62 未満の t (2 mod 未満の値どうしの積) なら、t - q mod は 2 mod 未満に収まる。
 constexpr inline u32 rem62(u64 t) const { return u32(t - u64((u128(t) * x) >> 64) * mod); }
 // u64 の全範囲の t で、q の誤差は 2 まで (t - q mod は 3 mod 未満) なので、2 mod 以上なら 2 mod を引いて 2 mod 未満にする。
 constexpr inline u32 rem64(u64 t) const {
  u32 r= u32(t - u64((u128(t) * x) >> 64) * mod);
  return r >= (mod << 1) ? r - (mod << 1) : r;
 }
};
