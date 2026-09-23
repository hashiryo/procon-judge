#pragma once
#include "common.hpp"
#include "mylib/number_theory/sum_on_primes.hpp"

// 素数の上での冪和 (Lucy DP) から、素数冪での値を与えて乗法的関数の総和を組む。
// 同じ和を Dirichlet 級数を経由せずに出す。
struct Solver {
  u128 n;
  i64 ans = 0;

  explicit Solver(u128 n) : n(n) {}

  void run() {
    Split s = split(n);
    const uint64_t r = s.r;
    Mint sum = s.tail;
    auto ps = sums_of_powers_on_primes<Mint>(r, 2);
    auto f = [&](Mint p, int e) { return p.pow(e - 1) * (p * (e + 1) - e); };
    auto g = [&](Mint p, int e) { return p.pow(e + e - 1) * (p * (e + 1) - e); };
    sum += multiplicative_sum<Mint>(2 * ps[1] - ps[0], f) * 3;
    sum += multiplicative_sum<Mint>(2 * ps[2] - ps[1], g) * 3;
    sum += Mint(r) * (r + 1) / 2;
    ans = sum.val();
  }

  i64 answer() const { return ans; }
};
