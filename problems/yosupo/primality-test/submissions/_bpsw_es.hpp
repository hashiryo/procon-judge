#pragma once
// 表を持たない BPSW (底 2 の強擬素数判定と extra strong Lucas 判定) の提出が共有する部品。
// Lucas 判定は Q = 1 で、P は 3, 4, 5, ... のうち (P^2 - 4 / n) = -1 となる最初のもの。
// 2^64 未満に反例が無いことは、Math::Prime::Util (0.31 から is_prime がこの形) が
// Feitsma の底 2 の擬素数の一覧で確かめている。
//
// P の選び方は、P <= 20 なら P^2 - 4 の素因数が 19 以下なので、n を 3, 5, ..., 19 で割った余りと
// n mod 8 から (p / n) を 8 個求め、最初の P を表で引く。8 個がどれも 1 のときだけ、
// 平方数かを確かめてから P = 21 以降を一般の Jacobi 記号で探す。
#include "../common.hpp"
#include "_mr_single.hpp"

namespace bpsw_es {

// a b R^{-1} + c mod n を (0, 3n) で返す。nc = n + c、0 <= c <= n、a b < 9 n^2 (n < 2^60 なら a, b < 3n で足りる)。
inline u64 mmul_add(u64 a, u64 b, u64 n, u64 ninv, u64 nc) {
  u128 t = (u128)a * b;
  u64 q = (u64)t * ninv;
  u64 m = (u64)(((u128)q * n) >> 64);
  return (u64)(t >> 64) + nc - m;
}

constexpr u32 kSmallPrimes[8] = {2, 3, 5, 7, 11, 13, 17, 19};
// P^2 - 4 を平方因子を除いて素因数分解したときの、kSmallPrimes の i 番目が出るかの bit。
constexpr u32 d_mask(u32 P) {
  u32 D = P * P - 4, m = 0;
  for (int i = 0; i < 8; ++i) {
    int e = 0;
    while (D % kSmallPrimes[i] == 0) D /= kSmallPrimes[i], ++e;
    if (e & 1) m |= 1u << i;
  }
  return D == 1 ? m : ~0u;
}
// 8 個の (p / n) = -1 の組 (bit) から、(P^2 - 4 / n) = -1 となる最初の P (3..20) を引く表。無ければ 0。
constexpr auto kFirstP = [] {
  std::array<u8, 256> t{};
  for (u32 j = 0; j < 256; ++j)
    for (u32 P = 3; P <= 20; ++P)
      if (std::popcount(d_mask(P) & j) & 1) {
        t[j] = (u8)P;
        break;
      }
  return t;
}();
static_assert(d_mask(20) != ~0u && d_mask(19) != ~0u && d_mask(17) != ~0u);

// (p / n) = -1 なら 1。p は奇素数、n は奇数で p で割り切れない。
template <u32 p> inline u32 jneg(u64 n) {
  constexpr u64 qr = [] {
    u64 m = 0;
    for (u32 x = 1; x < p; ++x) m |= 1ull << (x * x % p);
    return m;
  }();
  return (u32)(~qr >> (n % p) & 1) ^ (u32)((p & 3) == 3 && (n & 3) == 3);
}

// Jacobi 記号 (a / n)。n は正の奇数。P = 21 以降を探すときだけ使う。
inline int jacobi(u64 a, u64 n) {
  int t = 1;
  a %= n;
  while (a) {
    int z = __builtin_ctzll(a);
    a >>= z;
    if ((z & 1) && ((n & 7) == 3 || (n & 7) == 5)) t = -t;
    if ((a & n & 3) == 3) t = -t;
    u64 r = n % a;
    n = a, a = r;
  }
  return n == 1 ? t : 0;
}

inline bool is_square(u64 n) {
  u64 r = (u64)std::sqrt((double)n);
  if (r > 0xffffffffull) r = 0xffffffffull;
  while (r * r > n) --r;
  while (r < 0xffffffffull && (r + 1) * (r + 1) <= n) ++r;
  return r * r == n;
}

// Lucas 判定の P を返す。n が平方数か、P^2 - 4 と共通の素因数を持てば (合成数なので) 0 を返す。
// n は奇数で 41^2 以上、37 以下の素数で割り切れない。
inline u64 select_P(u64 n) {
  const u32 j = (u32)(0x28 >> (n & 7) & 1) | jneg<3>(n) << 1 | jneg<5>(n) << 2 | jneg<7>(n) << 3 | jneg<11>(n) << 4 | jneg<13>(n) << 5 | jneg<17>(n) << 6 | jneg<19>(n) << 7;
  u64 P = kFirstP[j];
  if (P) return P;
  if (is_square(n)) return 0;
  for (P = 21;; ++P) {
    const int jj = jacobi(P * P - 4, n);
    if (jj == -1) return P;
    if (jj == 0 && (P * P - 4) % n != 0) return 0;
  }
}

}  // namespace bpsw_es
