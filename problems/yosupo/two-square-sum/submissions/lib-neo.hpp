#pragma once
// N = a^2 + b^2 の組を全部出す。
//
// N を Z[i] で分解する。p = 3 (mod 4) の指数が奇数なら解なし。p = 1 (mod 4) は Cornacchia で
// pi = x + y i (norm p) に分け、pi^k conj(pi)^(e-k) (k = 0..e) を掛け合わせて作る。2 は (1 + i)、
// 残りの平方因子 S は最後にまとめて掛ける。組は (a, b) と (b, a) の両方を出し、重複を除く。
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include "neo/algo/compute/nt/factorize.hpp"

namespace two_square_internal {
using u64 = unsigned long long;
using i64 = long long;
using i128 = __int128_t;

inline u64 isqrt(u64 n) {
  u64 r = (u64)std::sqrt((double)n);
  while (r && r > n / r) --r;
  while ((r + 1) <= n / (r + 1)) ++r;
  return r;
}

inline u64 pow_mod(u64 a, u64 e, u64 m) {
  u64 r = 1 % m;
  a %= m;
  while (e) {
    if (e & 1) r = (u64)((i128)r * a % m);
    a = (u64)((i128)a * a % m);
    e >>= 1;
  }
  return r;
}

// p = 1 (mod 4) を x^2 + y^2 に分ける (x >= y >= 0)。Cornacchia。
inline std::pair<u64, u64> cornacchia(u64 p) {
  // -1 の平方根: 非剰余 g の g^((p-1)/4)。
  u64 g = 2;
  while (pow_mod(g, (p - 1) / 2, p) != p - 1) ++g;
  u64 r = pow_mod(g, (p - 1) / 4, p);
  u64 a = p, b = r;
  while (b && b > isqrt(p)) {
    std::swap(a, b);
    b %= a;
  }
  // ここで b <= sqrt(p)、p - b^2 は平方。
  const u64 c = isqrt(p - b * b);
  return {c, b};
}

struct G {
  i64 a, b;
};
inline G mul(G x, G y) {
  return {(i64)((i128)x.a * y.a - (i128)x.b * y.b),
          (i64)((i128)x.a * y.b + (i128)x.b * y.a)};
}

}  // namespace two_square_internal

struct Solver {
  u64 n;
  std::vector<std::pair<u64, u64>> res;

  explicit Solver(u64 n_) : n(n_) {}

  void run() {
    using namespace two_square_internal;
    res.clear();
    if (n == 0) {
      res.emplace_back(0, 0);
      return;
    }
    auto fac = factorize_pairs(n);
    u64 e2 = 0, sq = 1;
    for (auto &[p, e] : fac) {
      if (p == 2) {
        e2 = (u64)e;
      } else if (p % 4 == 3) {
        if (e % 2) return;  // 解なし
        for (int k = 0; k < e / 2; ++k) sq *= p;
      }
    }
    for (int k = 0; k < (int)(e2 / 2); ++k) sq *= 2;

    std::vector<G> z{{1, 0}};
    if (e2 % 2) z[0] = mul(z[0], G{1, 1});
    for (auto &[p, e] : fac) {
      if (p % 4 != 1) continue;
      auto [x, y] = cornacchia(p);
      // pi = x + y i の k 乗と conj の e-k 乗の積を k = 0..e で作る。
      std::vector<G> pw((size_t)e + 1), cw((size_t)e + 1);
      pw[0] = cw[0] = G{1, 0};
      for (int k = 1; k <= e; ++k) {
        pw[k] = mul(pw[k - 1], G{(i64)x, (i64)y});
        cw[k] = mul(cw[k - 1], G{(i64)x, -(i64)y});
      }
      std::vector<G> nx;
      nx.reserve(z.size() * (size_t)(e + 1));
      for (auto &v : z)
        for (int k = 0; k <= e; ++k) nx.push_back(mul(v, mul(pw[k], cw[e - k])));
      z.swap(nx);
    }
    for (auto &v : z) {
      const u64 a = (u64)(v.a < 0 ? -v.a : v.a) * sq;
      const u64 b = (u64)(v.b < 0 ? -v.b : v.b) * sq;
      res.emplace_back(a, b);
      if (a != b) res.emplace_back(b, a);
    }
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
  }

  const std::vector<std::pair<u64, u64>> &answer() const { return res; }
};
