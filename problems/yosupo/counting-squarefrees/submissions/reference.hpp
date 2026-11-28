#pragma once
// n 以下の無平方数の個数 Q(n) = sum_{d<=sqrt(n)} mu(d) floor(n/d^2)。
//
// sqrt(n) は 1e18 で 1e9 になり、mu をそこまで篩えない。D = min(sqrt(n), 1e7) で切り、
// d > D の分は v = floor(n/d^2) ごとにまとめる。A_v = M(floor(sqrt(n/v))) とすると
//   sum_{d>D} mu(d) floor(n/d^2) = sum_{v=1}^{V} A_v - V M(D)   (V = floor(n/(D+1)^2))
// になる (部分和分)。M は D 以下が篩の累積和、それより上は
//   M(y) = 1 - sum_{i=2}^{y} M(floor(y/i))
// を商の区間ごとに足してメモする。y > D では floor(y/i) > D になる i が y/D 個しかないので速い。
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace squarefree_internal {
using i64 = long long;
using u64 = unsigned long long;

inline u64 isqrt(u64 x) {
  if (x == 0) return 0;
  u64 r = (u64)std::sqrt((double)x);
  while (r && r > x / r) --r;
  while ((r + 1) <= x / (r + 1)) ++r;
  return r;
}

struct Core {
  u64 D = 0;
  std::vector<signed char> mu;
  std::vector<int> sm;  // M(x) for x <= D
  std::unordered_map<u64, i64> memo;

  void build(u64 D_) {
    D = D_;
    mu.assign(D + 1, 0);
    std::vector<int> primes;
    std::vector<signed char> comp(D + 1, 0);
    mu[1] = 1;
    for (u64 i = 2; i <= D; ++i) {
      if (!comp[i]) primes.push_back((int)i), mu[i] = -1;
      for (int p : primes) {
        if ((u64)p * i > D) break;
        comp[p * i] = 1;
        if (i % (u64)p == 0) {
          mu[p * i] = 0;
          break;
        }
        mu[p * i] = (signed char)-mu[i];
      }
    }
    sm.assign(D + 1, 0);
    int s = 0;
    for (u64 i = 1; i <= D; ++i) s += mu[i], sm[i] = s;
  }

  i64 mertens(u64 y) {
    if (y <= D) return sm[y];
    auto it = memo.find(y);
    if (it != memo.end()) return it->second;
    i64 s = 1;
    for (u64 i = 2, j; i <= y; i = j + 1) {
      const u64 q = y / i;
      j = y / q;
      s -= (i64)(j - i + 1) * mertens(q);
    }
    memo.emplace(y, s);
    return s;
  }

  u64 solve(u64 n, u64 cap = 10000000ULL) {
    if (n == 0) return 0;
    const u64 R = isqrt(n);
    const u64 lim = std::min<u64>(R, cap);
    build(lim);
    i64 total = 0;
    for (u64 d = 1; d <= lim; ++d) {
      if (!mu[d]) continue;
      total += (i64)mu[d] * (i64)(n / (d * d));
    }
    if (R > lim) {
      const u64 D1 = lim + 1;
      const u64 V = n / (D1 * D1);
      i64 sumA = 0;
      for (u64 v = 1; v <= V; ++v) sumA += mertens(isqrt(n / v));
      total += sumA - (i64)V * (i64)sm[lim];
    }
    return (u64)total;
  }
};

}  // namespace squarefree_internal

struct Solver {
  u64 n;
  u64 res = 0;
  squarefree_internal::Core core;

  explicit Solver(u64 n_) : n(n_) {}
  void run() { res = core.solve(n); }
  u64 answer() const { return res; }
};
