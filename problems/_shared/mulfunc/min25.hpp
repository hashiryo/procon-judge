#pragma once
// 乗法的な f (f(p^e) = a e + b p) の総和 sum_{n<=N} f(n) mod P を min_25 篩で求める。
//
// 第 1 段は Lucy_Hedgehog。商の列 Q_N の上で、初めすべての n を素数と見て
//   cnt(x) = (x 以下の素数の個数)、sm(x) = (x 以下の素数の和)
// を篩で作り、g(x) = a cnt(x) + b sm(x) = sum_{p<=x} f(p) にする。
// 第 2 段は最小の素因数の順の再帰。
//   S(x, k) = sum_{n<=x, n の最小の素因数が p_k より大きい, n>1} f(n)
//          = (g(x) - sum_{i<k} f(p_i)) + sum_{j>=k, p_j^2<=x} sum_{e>=1, p_j^{e+1}<=x} ( f(p_j^e) S(x/p_j^e, j+1) + f(p_j^{e+1}) )
// 答えは 1 + S(N, 0)。x は常に N の床の値なので、g は Q_N の表を引くだけでよい。
// 添字は x <= K なら x-1、x > K なら K + N/x - 1 (K = floor(sqrt(N)))。
//
// 速さのために 3 つ手を入れてある。
//  - 篩の添字は、i p <= K の範囲を掛け算 (K + i p - 1) で作り、それより上は double の割り算で作る。
//    小さい値の側 (v / p) も double で割る。どちらも値が 2^53 未満なので厳密。
//  - mod の掛け算は Barrett (P < 2^29 なので、P^2 未満の値なら 1 回の補正で足りる)。
//  - 再帰の内側で x / p を 1 度だけ割る。
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mf_min25 {
using u32 = unsigned;
using u64 = unsigned long long;
using u128 = unsigned __int128;

inline u64 isqrt(u64 x) {
  if (x == 0) return 0;
  u64 r = (u64)std::sqrt((double)x);
  while (r && r > x / r) --r;
  while ((r + 1) <= x / (r + 1)) ++r;
  return r;
}

struct Core {
  u64 n = 0, K = 0;
  u32 P = 1, a = 0, b = 0;
  u64 M = 0;  // Barrett: floor(2^64 / P)
  size_t len = 0, np = 0;
  std::vector<u32> cnt, sm, g, gp;
  std::vector<u32> primes;

  inline size_t idx(u64 x) const { return x <= K ? (size_t)x - 1 : K + (size_t)(n / x) - 1; }
  // v < P^2 (P^2 の 2 倍まで可) を mod P にする。
  inline u32 red(u64 v) const {
    const u64 q = (u64)(((u128)v * M) >> 64);
    u64 r = v - q * P;
    if (r >= P) r -= P;
    if (r >= P) r -= P;
    return (u32)r;
  }
  inline u32 fp(u64 p, int e) const { return red((u64)a * (u32)e + (u64)b * p); }  // p < P

  void build(u64 n_, u32 a_, u32 b_, u32 mod) {
    n = n_;
    P = mod;
    M = (~0ull) / P;
    a = a_ % P;
    b = b_ % P;
    const u32 inv2 = (u32)((P + 1) / 2);
    K = isqrt(n);
    const u64 m = n / (K + 1);
    len = (size_t)(K + m);
    cnt.assign(len, 0);
    sm.assign(len, 0);
    for (u64 v = 1; v <= K; ++v) {
      cnt[v - 1] = (u32)((v - 1) % P);
      sm[v - 1] = (u32)(((u128)(v % P) * ((v + 1) % P) % P * inv2 % P + P - 1) % P);
    }
    for (u64 i = 1; i <= m; ++i) {
      const u64 x = n / i;
      const size_t ix = K + (size_t)i - 1;
      cnt[ix] = (u32)((x - 1) % P);
      sm[ix] = (u32)(((u128)(x % P) * ((x + 1) % P) % P * inv2 % P + P - 1) % P);
    }
    for (u64 p = 2; p <= K; ++p) {
      if (cnt[p - 1] == cnt[p - 2]) continue;
      primes.push_back((u32)p);
      const u64 sp = cnt[p - 2], sp2 = sm[p - 2];
      const u64 p2 = p * p, pm = p % P;
      const u64 imax = std::min(n / (K + 1), n / p2);
      const u64 by_mul = std::min(m / p, imax);  // i p <= m のときだけ添字を掛け算で作れる
      const double dn = (double)n, dp = (double)p;
      for (u64 i = 1; i <= by_mul; ++i) {
        const size_t ix = K + (size_t)i - 1, jx = K + (size_t)(i * p) - 1;
        const u64 tc = (u64)cnt[jx] + P - sp;
        const u64 ts = (u64)sm[jx] + P - sp2;
        const u64 dc = tc >= P ? tc - P : tc;
        const u64 ds = ts >= P ? ts - P : ts;
        const u64 vc = (u64)cnt[ix] + P - dc;
        const u64 vs = (u64)sm[ix] + P - red(ds * pm);
        cnt[ix] = (u32)(vc >= P ? vc - P : vc);
        sm[ix] = (u32)(vs >= P ? vs - P : vs);
      }
      for (u64 i = by_mul + 1; i <= imax; ++i) {
        const size_t ix = K + (size_t)i - 1;
        const u64 xp = (u64)(dn / ((double)i * dp));
        const size_t jx = xp <= K ? (size_t)xp - 1 : K + (size_t)(dn / (double)xp) - 1;
        const u64 tc = (u64)cnt[jx] + P - sp;
        const u64 ts = (u64)sm[jx] + P - sp2;
        const u64 dc = tc >= P ? tc - P : tc;
        const u64 ds = ts >= P ? ts - P : ts;
        const u64 vc = (u64)cnt[ix] + P - dc;
        const u64 vs = (u64)sm[ix] + P - red(ds * pm);
        cnt[ix] = (u32)(vc >= P ? vc - P : vc);
        sm[ix] = (u32)(vs >= P ? vs - P : vs);
      }
      // 小さい値: v / p = j になる v をまとめて回す。j ごとに 1 度だけ引けばよい。
      for (u64 j = K / p; j >= p; --j) {
        const u64 tc = (u64)cnt[j - 1] + P - sp;
        const u64 ts = (u64)sm[j - 1] + P - sp2;
        const u64 dc = tc >= P ? tc - P : tc;
        const u64 ds = ts >= P ? ts - P : ts;
        const u32 dcr = (u32)dc, dsr = red(ds * pm);
        const u64 vlo = j * p, vhi = std::min(K, vlo + p - 1);
        for (u64 v = vhi; v >= vlo; --v) {
          const u64 vc = (u64)cnt[v - 1] + P - dcr;
          const u64 vs = (u64)sm[v - 1] + P - dsr;
          cnt[v - 1] = (u32)(vc >= P ? vc - P : vc);
          sm[v - 1] = (u32)(vs >= P ? vs - P : vs);
        }
        if (j == p) break;
      }
    }
    np = primes.size();
    g.resize(len);
    for (size_t i = 0; i < len; ++i) g[i] = red((u64)a * cnt[i] + (u64)b * sm[i]);
    gp.assign(np + 1, 0);
    for (size_t k = 0; k < np; ++k) gp[k + 1] = red((u64)gp[k] + fp(primes[k], 1));
  }

  u32 dfs(u64 x, size_t k) const {
    u64 ret = (u64)g[idx(x)] + P - gp[k];
    if (ret >= P) ret -= P;
    for (size_t j = k; j < np; ++j) {
      const u64 p = primes[j];
      if (p * p > x) break;
      const u64 xp = x / p;
      u64 pe = p;
      for (int e = 1; pe <= xp; ++e) {
        ret += (u64)fp(p, e) * dfs(x / pe, j + 1) + fp(p, e + 1);
        if (ret >= (1ull << 63)) ret %= P;
        pe *= p;
      }
    }
    return (u32)(ret % P);
  }

  u32 solve() {
    if (n == 0) return 0;
    return (u32)((1 + (u64)dfs(n, 0)) % P);
  }
};

}  // namespace mf_min25
