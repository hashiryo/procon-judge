#pragma once
// 商の列 Q_N の上で、Dirichlet 積 h = f * g の累積和 H(x) = sum_{ij<=x} f(i) g(j) を
// O(N^{2/3}) で求める。入力は Q_N の昇順に並べた F(x), G(x)。
//
// 方針は「積の小さい組を先に潰して、残りを商の列の上で数える」。
//
// 1. x <= K (K = floor(sqrt(N))) は h(n) = sum_{d|n} f(d) g(n/d) を約数の上で畳んで
//    累積和にする (O(K log K))。
// 2. ij <= K の組は、どの x にも同じだけ効く (N/(ij) >= K) ので 1 に入っている。
//    それより大きい組のうち、i <= N^{1/3} かつ j <= sqrt(N/i) のものは、組ごとに
//    d = floor(N/(ij)) を 1 つ決めて changes[d] に f(i)g(j) + f(j)g(i) を足す。
//    (i, j) の組が x に効くのは x <= N/(ij) のときなので、d から下へ running に足していけば
//    すべての x に一度で配れる。組の数は sum_{i<=N^{1/3}} sqrt(N/i) = 2N^{2/3}。
// 3. 残りは x ごとに双曲線の形で数える。x = floor(N/d) として
//      H(x) = sum_{i<=sqrt(x)} f(i)G(x/i) + sum_{j<=sqrt(x)} g(j)F(x/j) - F(sqrt x)G(sqrt x)
//    の i (と j) を、changes に入れた範囲の外側だけ回す。x ごとの回数は
//    min(sqrt(x), x/(d+1)) で、d について足すと 2N^{2/3} になる。
//
// 各 x の答えは H(N/d) = running(それ以下の d' の changes の和) + 双曲線の残り + 端の補正。
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mf_dirichlet {
using u32 = unsigned;
using u64 = unsigned long long;

inline u64 isqrt(u64 x) {
  if (x == 0) return 0;
  u64 r = (u64)std::sqrt((double)x);
  while (r && r > x / r) --r;
  while ((r + 1) <= x / (r + 1)) ++r;
  return r;
}

// f, g: Q_N 昇順の累積和。戻り値: h = f * g の累積和 (Q_N 昇順)。
inline std::vector<u32> convolve(u64 n, const std::vector<u32> &f,
                                 const std::vector<u32> &g, u32 P) {
  if (n == 0) return {};
  const u64 K = isqrt(n);
  const size_t large = (size_t)(n / (K + 1));
  const size_t len = (size_t)K + large;
  std::vector<u32> res(len, 0);
  if (len == 0) return res;

  // f(n), g(n) (n <= K) と、K 以下の累積和の答え
  std::vector<u32> fv(K + 2, 0), gv(K + 2, 0);
  for (u64 v = 1; v <= K; ++v) {
    fv[v] = (u32)(((u64)f[v - 1] + P - (v > 1 ? f[v - 2] : 0)) % P);
    gv[v] = (u32)(((u64)g[v - 1] + P - (v > 1 ? g[v - 2] : 0)) % P);
  }
  std::vector<u32> hv(K + 1, 0);
  for (u64 d = 1; d <= K; ++d) {
    if (!fv[d]) continue;
    for (u64 m = 1; d * m <= K; ++m) hv[d * m] = (u32)((hv[d * m] + (u64)fv[d] * gv[m]) % P);
  }
  u64 s = 0;
  for (u64 v = 1; v <= K; ++v) {
    s += hv[v];
    if (s >= P) s -= P;
    res[v - 1] = (u32)s;
  }
  if (large == 0) return res;

  const u64 f1 = fv[1], g1 = gv[1];
  // changes[d-1]: floor(N/(ij)) = d になる組の寄与
  std::vector<u32> chg(large, 0);

  // 2. 大きい組を散らす
  for (u64 i = 2; i <= K; ++i) {
    const u64 m = n / i;
    const u64 split = isqrt(m);
    if (i > split) break;
    const u64 fi = fv[i], gi = gv[i];
    if (i > K / i) {
      const u64 d = m / i;  // N / i^2
      if (d >= 1 && d <= large) chg[d - 1] = (u32)((chg[d - 1] + fi * gi) % P);
    }
    const u64 start = std::max(i, K / i) + 1;
    for (u64 j = start; j <= split; ++j) {
      const u64 d = m / j;
      if (d < 1 || d > large) continue;
      const u64 fj = fv[j], gj = gv[j];
      chg[d - 1] = (u32)((chg[d - 1] + fi * gj + fj * gi) % P);
    }
    const u64 row = m / (split + 1);
    if (row >= 1 && row <= large) {
      const u64 pf = f[split - 1], pg = g[split - 1];
      const u64 pgi = g[i - 2], pfi = f[i - 1];
      const u64 counted = (fi * ((pg + P - pgi) % P) + ((pf + P - pfi) % P) * gi) % P;
      res[len - row] = (u32)(((u64)res[len - row] + P - counted) % P);
    }
  }

  // 3. 商の列を大きいほうから
  const u64 pfK = f[K - 1], pgK = g[K - 1];
  u64 running = ((u64)res[K - 1] + P - (f1 * pgK + pfK * g1) % P) % P;
  running = (running + f1 * g1) % P;
  for (u64 d = large; d >= 1; --d) {
    const u64 q = n / d;
    const u64 end = std::min(isqrt(q), q / (d + 1));
    const u64 upper = std::min(large / d, end);
    running = (running + chg[d - 1]) % P;
    running = (running + res[len - d]) % P;
    u64 sum = 0;
    for (u64 i = 2; i <= upper; ++i) {
      const size_t j = len - (size_t)(d * i);
      sum += (u64)fv[i] * g[j] + (u64)f[j] * gv[i];
      if (sum >= (1ull << 63)) sum %= P;
    }
    for (u64 i = upper + 1; i <= end; ++i) {
      const u64 j = q / i;
      sum += (u64)fv[i] * g[j - 1] + (u64)f[j - 1] * gv[i];
      if (sum >= (1ull << 63)) sum %= P;
    }
    const u64 pf = f[end - 1], pg = g[end - 1];
    u64 value = (running + P - pf * pg % P) % P;
    value = (value + sum) % P;
    value = (value + (u64)f[len - d] * g1) % P;
    res[len - d] = (u32)((value + f1 * (u64)g[len - d]) % P);
  }
  return res;
}

inline u32 pow_mod(u32 a, u64 e, u32 P) {
  u64 r = 1, b = a;
  while (e) {
    if (e & 1) r = r * b % P;
    b = b * b % P;
    e >>= 1;
  }
  return (u32)r;
}

// f の Dirichlet 逆元 g の累積和を Q_N 上で求める。f は Q_N 昇順の累積和。
inline std::vector<u32> inverse(u64 n, const std::vector<u32> &f, u32 P) {
  if (n == 0) return {};
  const u64 K = isqrt(n);
  const size_t large = (size_t)(n / (K + 1));
  const size_t len = (size_t)K + large;
  std::vector<u32> res(len, 0);
  std::vector<u32> fv(K + 2, 0), gv(K + 2, 0);
  for (u64 v = 1; v <= K; ++v) fv[v] = (u32)(((u64)f[v - 1] + P - (v > 1 ? f[v - 2] : 0)) % P);
  const u32 f1 = fv[1];
  const u32 f1inv = pow_mod(f1, P - 2, P);
  // g(1) = 1/f(1)。g(i) は -1/f(1) に、自分より大きい約数の寄与を掛けたもの。
  gv[1] = f1inv;
  for (u64 i = 1; i <= K; ++i) {
    if (i > 1) gv[i] = (u32)((u64)((P - gv[i]) % P) * f1inv % P);
    const u64 w = gv[i];
    if (!w) continue;
    for (u64 j = 2, m = i + i; m <= K; ++j, m += i) {
      gv[m] = (u32)((gv[m] + w * fv[j]) % P);
    }
  }
  u64 s = 0;
  for (u64 v = 1; v <= K; ++v) {
    s += gv[v];
    if (s >= P) s -= P;
    res[v - 1] = (u32)s;
  }
  if (large == 0) return res;

  const u64 g1 = gv[1];
  std::vector<u32> chg(large, 0);
  for (u64 i = 2; i <= K; ++i) {
    const u64 m = n / i;
    const u64 split = isqrt(m);
    if (i > split) break;
    const u64 fi = fv[i], gi = gv[i];
    if (i > K / i) {
      const u64 d = m / i;
      if (d >= 1 && d <= large) chg[d - 1] = (u32)((chg[d - 1] + fi * gi) % P);
    }
    const u64 start = std::max(i, K / i) + 1;
    for (u64 j = start; j <= split; ++j) {
      const u64 d = m / j;
      if (d < 1 || d > large) continue;
      const u64 fj = fv[j], gj = gv[j];
      chg[d - 1] = (u32)((chg[d - 1] + fi * gj + fj * gi) % P);
    }
    const u64 row = m / (split + 1);
    if (row >= 1 && row <= large) {
      const u64 pf = f[split - 1], pg = res[split - 1];
      const u64 pgi = res[i - 2], pfi = f[i - 1];
      const u64 counted = (fi * ((pg + P - pgi) % P) + ((pf + P - pfi) % P) * gi) % P;
      res[len - row] = (u32)(((u64)res[len - row] + P - counted) % P);
    }
  }
  const u64 pfK = f[K - 1], pgK = res[K - 1];
  u64 running = (P - (f1 * pgK + pfK * g1) % P) % P;
  running = (running + f1 * g1) % P;
  for (u64 d = large; d >= 1; --d) {
    const u64 q = n / d;
    const u64 end = std::min(isqrt(q), q / (d + 1));
    const u64 upper = std::min(large / d, end);
    running = (running + chg[d - 1]) % P;
    running = (running + res[len - d]) % P;
    u64 sum = 0;
    for (u64 i = 2; i <= upper; ++i) {
      const size_t j = len - (size_t)(d * i);
      sum += (u64)fv[i] * res[j] + (u64)f[j] * gv[i];
      if (sum >= (1ull << 63)) sum %= P;
    }
    for (u64 i = upper + 1; i <= end; ++i) {
      const u64 j = q / i;
      sum += (u64)fv[i] * res[j - 1] + (u64)f[j - 1] * gv[i];
      if (sum >= (1ull << 63)) sum %= P;
    }
    const u64 pf = f[end - 1], pg = res[end - 1];
    u64 value = (running + P - pf * pg % P) % P;
    value = (value + sum) % P;
    value = (value + (u64)f[len - d] * g1) % P;
    res[len - d] = (u32)((u64)f1inv * ((P - value) % P) % P);
  }
  return res;
}

}  // namespace mf_dirichlet
