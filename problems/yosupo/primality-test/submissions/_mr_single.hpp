#pragma once
// 1 回の呼び出しの速さ (latency) を詰めた Miller-Rabin の核。
// - 値は Montgomery 表現 (R = 2^64) で持ち、[0, 2n) に収まっていれば正規化しない。
//   還元の最後の条件付きの引き算が鎖から消える。
// - 底 2 は指数を上の桁から読み、ビットが立つ段は片方の因数を 2 倍してから掛ける。
//   x^2 2^bit を掛け算 1 回で求められるので、底 2 の鎖は掛け算が 2 乗だけになる。
// - ほかの底は指数を下の桁から読み、ビットで掛ける相手を 1 と選ぶので分岐しない。
//   2 乗の鎖と積の鎖が別々に進み、鎖の長さは指数の桁数で済む。
// - 全部の底を 1 つのループで同時に回し、鎖どうしを重ねる。
#include "../common.hpp"

namespace mr_single {

// n * inv64(n) ≡ 1 (mod 2^64)。n は奇数。
inline u64 inv64(u64 n) {
  u64 x = (3 * n) ^ 2;  // 下位 5 bit が正しい
  u64 y = 1 - n * x;
  x *= 1 + y, y *= y;
  x *= 1 + y, y *= y;
  x *= 1 + y, y *= y;
  x *= 1 + y;
  return x;
}

// a b R^{-1} mod n を (0, 2n) で返す。a b < n R が前提。
inline u64 mmul(u64 a, u64 b, u64 n, u64 ninv) {
  u128 t = (u128)a * b;
  u64 q = (u64)t * ninv;
  u64 m = (u64)(((u128)q * n) >> 64);
  return (u64)(t >> 64) + n - m;
}

// b R mod n を [0, 2n) で返す。one = R mod n、inv_n は 1/n の近似。b は n 未満で 2^32 未満。
// 商 b one / n (b 未満) を浮動小数点で見積もる。見積もりは真の商から ±1 しかずれない。
inline u64 to_mont_small(u64 b, u64 one, u64 n, double inv_n) {
  u64 q = (u64)(i64)((double)(i64)one * (double)(i64)b * inv_n);
  u64 r = b * one - q * n;
  return (i64)r < 0 ? r + n : r;
}

// 底 2 と底 bs[0], ..., bs[K-1] のすべての強擬素数判定に通るか。
// n は奇数で 3 <= n < 2^61。bs[k] は 2 以上で、n 未満かつ 2^32 未満。
// n < 2^61 は底 2 の段で 2 倍した因数を掛けても a b < n R を保つため。
template <int K> inline bool sprp(u64 n, const u64 (&bs)[K]) {
  const u64 ninv = inv64(n), one = (0 - n) % n, mone = n - one;
  const double inv_n = 1.0 / (double)(i64)n;
  const int s = __builtin_ctzll(n - 1);
  const u64 d = (n - 1) >> s;
  const int L = 64 - __builtin_clzll(d);
  // 底 2: 最上位のビット (L-1 桁目) は x = 2 で済ませ、L-2 桁目から下へ読む。
  u64 x = one << 1, da = d << (64 - L);
  // ほかの底: d は奇数なので 0 桁目は y = b で済ませ、1 桁目から上へ読む。
  u64 y[K], z[K];
#pragma GCC unroll 8
  for (int k = 0; k < K; ++k) y[k] = to_mont_small(bs[k], one, n, inv_n), z[k] = mmul(y[k], y[k], n, ninv);
  u64 db = d >> 1;
  for (int i = 1; i < L - 1; ++i) {
    da <<= 1;
    x = mmul(x, x << (da >> 63), n, ninv);
    const u64 bit = db & 1;
    db >>= 1;
#pragma GCC unroll 8
    for (int k = 0; k < K; ++k) y[k] = mmul(y[k], bit ? z[k] : one, n, ninv), z[k] = mmul(z[k], z[k], n, ninv);
  }
  if (L > 1) {
    x = mmul(x, x << 1, n, ninv);  // 0 桁目は 1
#pragma GCC unroll 8
    for (int k = 0; k < K; ++k) y[k] = mmul(y[k], z[k], n, ninv);  // L-1 桁目は 1
  }
  u64 v[K + 1];
  v[0] = x;
#pragma GCC unroll 8
  for (int k = 0; k < K; ++k) v[k + 1] = y[k];
  unsigned pass = 0;
#pragma GCC unroll 8
  for (int k = 0; k <= K; ++k) {
    v[k] = v[k] >= n ? v[k] - n : v[k];
    pass |= unsigned(v[k] == one || v[k] == mone) << k;
  }
  constexpr unsigned all = (1u << (K + 1)) - 1;
  for (int r = 1; pass != all && r < s; ++r)
#pragma GCC unroll 8
    for (int k = 0; k <= K; ++k) {
      v[k] = mmul(v[k], v[k], n, ninv);
      v[k] = v[k] >= n ? v[k] - n : v[k];
      pass |= unsigned(v[k] == mone) << k;
    }
  return pass == all;
}

// n が奇素数 p で割り切れるか。p の 2^64 を法とする逆元を掛け、(2^64-1)/p 以下なら割り切れる。
template <u64 p> inline u32 divisible(u64 n) {
  constexpr u64 inv = [] {
    u64 x = p;
    for (int i = 0; i < 6; ++i) x *= 2 - p * x;
    return x;
  }();
  return n * inv <= ~0ull / p;
}

// 64 未満は表で、偶数と 37 以下の奇素数の倍数は割り算で落とす。
// 決着すれば 0 (合成数) か 1 (素数)、決着しなければ -1 を返す。
inline int small_check(u64 n) {
  if (n < 64) return 0x28208a20a08a28acull >> n & 1;
  if (n % 2 == 0) return 0;
  if (divisible<3>(n) | divisible<5>(n) | divisible<7>(n) | divisible<11>(n) | divisible<13>(n) | divisible<17>(n) | divisible<19>(n) | divisible<23>(n) | divisible<29>(n) | divisible<31>(n) | divisible<37>(n)) return 0;
  if (n < 41 * 41) return 1;
  return -1;
}

}  // namespace mr_single
