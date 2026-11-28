#pragma once
// 今の Library の min_25 篩。素数の上の 1 と p の和を Lucy_Hedgehog で出してから、
// 素数冪での値 f(p, e) = a e + b p で乗法的関数の総和に持ち上げる。
#include "common.hpp"
#include "mylib/number_theory/sum_on_primes.hpp"

struct Solver {
  u64 n, a, b;
  u32 ans = 0;

  Solver(u64 n_, u64 a_, u64 b_) : n(n_), a(a_), b(b_) {}

  void run() {
    if (n == 0) {
      ans = 0;
      return;
    }
    const Mint am = Mint(a), bm = Mint(b);
    auto ps = sums_of_powers_on_primes<Mint>(n, 1);  // ps[0] = sum 1, ps[1] = sum p
    // 素数の上では f(p) = a + b p。f(p^e) = a e + b p。
    auto f = [&](u64 p, int e) { return am * Mint(e) + bm * Mint(p); };
    ans = multiplicative_sum<Mint>(am * ps[0] + bm * ps[1], f).val();
  }

  u64 answer() const { return ans; }
};
