#pragma once
#include "pj.hpp"
#include <cstdint>
#include <memory>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 998244353 の Montgomery (R = 2^32)。値は 2p 未満で持つ。この問題の提出だけで使う。
namespace mont {
constexpr u32 P = 998244353, P2 = 2 * P;
constexpr u32 NINV = [] {
  u32 x = 1;
  for (int i = 0; i < 5; ++i) x *= 2 - P * x;
  return -x;
}();
constexpr u32 R1 = u32((u64(1) << 32) % P);
constexpr u32 R2 = u32(u64(R1) * R1 % P);
// t < 2^32 p なら p + t / 2^32 未満を返す。
inline u32 redc(u64 t) { return u32((t + u64(u32(t) * NINV) * P) >> 32); }
inline u32 mul(u32 a, u32 b) { return redc(u64(a) * b); }
inline u32 add(u32 a, u32 b) {
  const u32 s = a + b;
  return s >= P2 ? s - P2 : s;
}
// a b + c d を 1 回の還元で。積の和は 8 p^2 未満で、還元の結果は 3p 未満なので 2p を 1 回引く。
inline u32 mulsum(u32 a, u32 b, u32 c, u32 d) {
  const u32 r = redc(u64(a) * b + u64(c) * d);
  return r >= P2 ? r - P2 : r;
}
inline u32 enc(u64 x) { return mul(u32(x % P), R2); }
inline u32 dec(u32 x) {
  const u32 r = redc(x);
  return r >= P ? r - P : r;
}
}  // namespace mont

// T を n 個置ける 64 byte 境界の領域。1 MB 以上は 2 MB 境界に寄せて huge page を頼む (Linux)。
template <class T> struct HugeArray {
  unique_ptr<char[]> raw;
  T *p = nullptr;
  HugeArray() = default;
  explicit HugeArray(size_t n) {
    const size_t want = n * sizeof(T);
    const size_t A = want >= (size_t(1) << 20) ? (size_t(1) << 21) : 64;
    const size_t bytes = (want + A - 1) & ~(A - 1);
    raw.reset(new char[bytes + A]);
    p = reinterpret_cast<T *>((reinterpret_cast<uintptr_t>(raw.get()) + A - 1) & ~uintptr_t(A - 1));
#ifdef __linux__
    if (A > 64) madvise(p, bytes, MADV_HUGEPAGE);
#endif
  }
  T &operator[](size_t i) { return p[i]; }
  const T &operator[](size_t i) const { return p[i]; }
};
