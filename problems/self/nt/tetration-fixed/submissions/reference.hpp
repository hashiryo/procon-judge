// 期待出力を作る参照実装。今の Library の mod_tetration の算法 (指数を λ のような値で落とす) を、
// ライブラリを include しない形に書き出したものです。λ のような値は試し割りで作ります。
#pragma once
#include <numeric>
#include <utility>
#include <vector>

namespace reference_impl {
using u64 = unsigned long long;
using u128 = __uint128_t;

// m の λ のような値。2^e は e <= 3 なら φ = 2^(e-1)、e >= 4 なら 2^(e-2)、奇数の素数べきは φ で、
// 素数べきごとの値を lcm で束ねます。法は 998244353 から λ をたどった数十種類しか出ないので、
// 一度出した値は覚えておきます (試し割りは素数で sqrt(m) 回かかるため)。
inline u64 lambda_like_calc(u64 m) {
  u64 l = 1;
  for (u64 p = 2; p * p <= m; ++p) {
    if (m % p) continue;
    int e = 0;
    do m /= p, ++e;
    while (m % p == 0);
    u64 t = p - 1;
    const int ex = e - (p == 2 && e > 3 ? 1 : 0);
    for (int i = 1; i < ex; ++i) t *= p;
    l = l / std::gcd(l, t) * t;
  }
  if (m > 1) {
    const u64 t = m - 1;
    l = l / std::gcd(l, t) * t;
  }
  return l;
}

inline u64 lambda_like(u64 m) {
  static std::vector<std::pair<u64, u64>> memo;
  for (const auto& e : memo)
    if (e.first == m) return e.second;
  const u64 v = lambda_like_calc(m);
  memo.emplace_back(m, v);
  return v;
}

inline u64 pow_mod(u64 a, u64 e, u64 m) {
  u64 r = 1;
  for (a %= m; e; e >>= 1, a = (u64)((u128)a * a % m))
    if (e & 1) r = (u64)((u128)r * a % m);
  return r;
}

inline u64 tetration(u64 a, u64 b, u64 m) {
  if (m == 1) return 0;
  if (a == 0) return (b ^ 1) & 1;
  if (b == 0) return 1;
  if (b == 1) return a % m;
  const u64 l = lambda_like(m);
  u64 e = b == 2 ? a : tetration(a, b - 1, l);
  if (!e) e = l;
  return pow_mod(a, e, m);
}
}  // namespace reference_impl

struct Solver {
  unsigned solve(unsigned a, unsigned b) { return (unsigned)reference_impl::tetration(a, b, 998244353); }
};
