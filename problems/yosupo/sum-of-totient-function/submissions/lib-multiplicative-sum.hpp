#pragma once
#include "common.hpp"
#include "mylib/number_theory/sum_on_primes.hpp"

// 素数の上での冪乗和を Lucy_Hedgehog の方法で出してから、乗法的関数の和に
// 持ち上げる。O(N^(3/4) / log N) なので、ディリクレ級数より一段遅い。
struct Solver {
  i64 n;
  Mint ans;

  explicit Solver(i64 n) : n(n) {}

  void run() {
    auto ps = sums_of_powers_on_primes<Mint>(n, 1);
    auto f = [](i64 p, short e) { return Mint(p).pow(e - 1) * (p - 1); };
    ans = multiplicative_sum(ps[1] - ps[0], f);
  }

  i64 answer() const { return ans.val(); }
};
