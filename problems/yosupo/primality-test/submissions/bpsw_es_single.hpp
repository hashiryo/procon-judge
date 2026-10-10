#pragma once
// 表を持たない BPSW。底 2 の強擬素数判定と、extra strong Lucas 判定を 1 つのループで同時に回す。
// Lucas 判定は Q = 1 で、P は 3, 4, 5, ... のうち (P^2 - 4 / n) = -1 となる最初のもの。
// 2^64 未満に反例が無いことは、Math::Prime::Util (0.31 から is_prime がこの形) が
// Feitsma の底 2 の擬素数の一覧で確かめている。
//
// Lucas 側は V 列だけを梯子で求める。(V_k, V_{k+1}) から
//   V_{2k} = V_k^2 - 2、V_{2k+1} = V_k V_{k+1} - P、V_{2k+2} = V_{k+1}^2 - 2
// なので、1 段は 2 つの独立な積になる。定数の引き算は還元の最後の足し算にまとめるので、
// 1 段の鎖は積 1 回ぶん (と選択 1 回) で、底 2 の鎖 (_mr_single.hpp と同じ形) とほぼ同じ長さになる。
// U_d ≡ 0 は、Q = 1 では D U_d = 2 V_{d+1} - P V_d なので、V_d ≡ ±2 のとき V_{d+1} ≡ ±P と同じ。
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
  while (r * r > n) --r;
  while ((r + 1) * (r + 1) <= n) ++r;
  return r * r == n;
}

// n は奇数で 41^2 <= n < 2^60、37 以下の素数で割り切れない。
inline bool bpsw(u64 n) {
  const u32 j = (u32)(0x28 >> (n & 7) & 1) | jneg<3>(n) << 1 | jneg<5>(n) << 2 | jneg<7>(n) << 3 | jneg<11>(n) << 4 | jneg<13>(n) << 5 | jneg<17>(n) << 6 | jneg<19>(n) << 7;
  u64 P = kFirstP[j];
  if (P == 0) {
    if (is_square(n)) return false;
    for (P = 21;; ++P) {
      const int jj = jacobi(P * P - 4, n);
      if (jj == -1) break;
      if (jj == 0 && (P * P - 4) % n != 0) return false;  // n と P^2 - 4 に共通の素因数がある
    }
  }
  using mr_single::mmul;
  const u64 ninv = mr_single::inv64(n), one = (0 - n) % n, mone = n - one;
  const double inv_n = 1.0 / (double)(i64)n;
  const u64 two = 2 * one >= n ? 2 * one - n : 2 * one;
  u64 Pm = mr_single::to_mont_small(P, one, n, inv_n);
  Pm = Pm >= n ? Pm - n : Pm;
  const u64 nc2 = 2 * n - two, ncP = 2 * n - Pm;  // n + (-2 mod n)、n + (-P mod n)
  // 底 2 は (n-1) = dA 2^sA の dA を、Lucas は (n+1) = dL 2^sL の dL を、上の桁から読む。
  const int sA = __builtin_ctzll(n - 1), sL = __builtin_ctzll(n + 1);
  const u64 dA = (n - 1) >> sA, dL = (n + 1) >> sL;
  const int LA = 64 - __builtin_clzll(dA), LL = 64 - __builtin_clzll(dL);
  u64 da = dA << (64 - LA), dl = dL << (64 - LL);
  // 底 2: 最上位のビットは x = 2 で済ませる。
  u64 x = 2 * one;
  // Lucas: (V_k, V_{k+1}) = prev ? (a, s) : (s, a)。最上位のビットで k = 1、(V_1, V_2) = (P, P^2 - 2)。
  u64 a = Pm, s = mmul_add(Pm, Pm, n, ninv, nc2);
  u64 prev = 1;
  auto stepA = [&] {
    da <<= 1;
    x = mmul(x, x << (da >> 63), n, ninv);
  };
  auto stepL = [&] {
    dl <<= 1;
    const u64 b = dl >> 63;
    const u64 sel = b == prev ? s : a;
    const u64 a2 = mmul_add(a, s, n, ninv, ncP);  // V_{2k+1}
    s = mmul_add(sel, sel, n, ninv, nc2);          // V_{2k} か V_{2k+2}
    a = a2, prev = b;
  };
  const int common = (LA < LL ? LA : LL) - 1;
  for (int i = 0; i < common; ++i) stepA(), stepL();
  for (int i = common; i < LA - 1; ++i) stepA();
  for (int i = common; i < LL - 1; ++i) stepL();
  // 底 2 の判定
  x = x >= n ? x - n : x;
  bool pa = x == one || x == mone;
  for (int r = 1; !pa && r < sA; ++r) {
    if (x == one) return false;
    x = mmul(x, x, n, ninv);
    x = x >= n ? x - n : x;
    pa = x == mone;
  }
  if (!pa) return false;
  // extra strong Lucas の判定: (U_d ≡ 0 かつ V_d ≡ ±2) か、0 <= r < sL - 1 のどれかで V_{d 2^r} ≡ 0。
  auto norm3 = [n](u64 v) {  // (0, 3n) から [0, n) へ
    v = v >= n ? v - n : v;
    return v >= n ? v - n : v;
  };
  u64 V = norm3(prev ? a : s), W = norm3(prev ? s : a);
  if ((V == two && W == Pm) || (V == n - two && W == n - Pm)) return true;
  for (int r = 0; r < sL - 1; ++r) {
    if (V == 0) return true;
    if (V == two) return false;  // 以降はずっと 2
    V = norm3(mmul_add(V, V, n, ninv, nc2));
  }
  return false;
}

}  // namespace bpsw_es

inline bool is_prime_bpsw_es(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  return bpsw_es::bpsw(n);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_bpsw_es(qs[i]);
  return ans;
}
