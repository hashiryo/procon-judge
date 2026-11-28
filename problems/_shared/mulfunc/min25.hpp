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
// 2026-11-28 の実験: 今の Library の multiplicative_sum より遅い (N = 1e13 で手元の M2 の
// algo が 26.0 s 対 18.3 s。内訳は篩 15.9 s、再帰 10.1 s)。原因は篩の内側の整数の割り算で、
// Library は double の割り算と n*p の掛け算で添字を作っている。この写しは捨てる候補。
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
  size_t len = 0, np = 0;
  std::vector<u32> cnt, sm, g, gp;
  std::vector<u32> primes;

  size_t idx(u64 x) const {
    return x <= K ? (size_t)x - 1 : K + (size_t)(n / x) - 1;
  }

  // f(p^e) = a e + b p (mod P)
  u32 fp(u64 p, int e) const {
    return (u32)(((u64)a * (u32)e + (u64)b * (p % P)) % P);
  }

  void build(u64 n_, u32 a_, u32 b_, u32 mod) {
    n = n_;
    P = mod;
    a = a_ % P;
    b = b_ % P;
    const u32 inv2 = (u32)((P + 1) / 2);  // P は奇素数
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
    // Lucy_Hedgehog。x を大きい順に回すので、x/p はこの段の更新前の値を読む。
    for (u64 p = 2; p <= K; ++p) {
      if (cnt[p - 1] == cnt[p - 2]) continue;  // p は素数でない
      primes.push_back((u32)p);
      const u64 sp = cnt[p - 2], sp2 = sm[p - 2];  // pi(p-1), sum_{q<=p-1} q
      const u64 p2 = p * p;
      const u64 imax = std::min(n / (K + 1), n / p2);
      for (u64 i = 1; i <= imax; ++i) {
        const size_t ix = K + (size_t)i - 1;
        const u64 xp = (n / i) / p;
        const size_t jx = idx(xp);
        cnt[ix] = (u32)((cnt[ix] + P - (u64)((cnt[jx] + P - sp) % P)) % P);
        sm[ix] = (u32)((sm[ix] + P - (u64)((sm[jx] + P - sp2) % P) * (p % P) % P) % P);
      }
      for (u64 v = K; v >= p2; --v) {
        const size_t jx = (size_t)(v / p) - 1;
        cnt[v - 1] = (u32)((cnt[v - 1] + P - (u64)((cnt[jx] + P - sp) % P)) % P);
        sm[v - 1] = (u32)((sm[v - 1] + P - (u64)((sm[jx] + P - sp2) % P) * (p % P) % P) % P);
      }
    }
    np = primes.size();
    g.resize(len);
    for (size_t i = 0; i < len; ++i) {
      g[i] = (u32)(((u64)a * cnt[i] + (u64)b * sm[i]) % P);
    }
    gp.assign(np + 1, 0);
    for (size_t k = 0; k < np; ++k) {
      gp[k + 1] = (u32)(((u64)gp[k] + fp(primes[k], 1)) % P);
    }
  }

  u32 dfs(u64 x, size_t k) const {
    u64 ret = (g[idx(x)] + P - gp[k]) % P;
    for (size_t j = k; j < np; ++j) {
      const u64 p = primes[j];
      if (p > x / p) break;
      u64 pe = p;
      for (int e = 1; pe <= x / p; ++e) {
        const u32 fpe = fp(p, e);
        ret += (u64)fpe * dfs(x / pe, j + 1) % P + fp(p, e + 1);
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
