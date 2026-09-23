#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/number_theory/DirichletSeries.hpp"

// Dirichlet 級数。「m 以下の約数 i」の側を F、「x + i」の側を G として、積の係数の
// 総和を取る。O(N^(2/3) log^(1/3) N)。
struct Solver {
  i64 n, m, ans = 0;

  Solver(i64 n, i64 m) : n(n), m(m) {}

  void run() {
    using Mint = ModInt<998244353>;
    using DS = DirichletSeries<Mint>;
    const i64 mm = m * (m + 1) / 2;
    auto fsum = [&](int x) {
      if (x < m) return (i64)x * (x + 1) / 2;
      return mm;
    };
    auto gsum = [&](int x) { return (i64)x * (x + 3) / 2; };
    DS f(n, fsum), g(n, gsum);
    ans = (f * g).sum().val();
  }

  i64 answer() const { return ans; }
};
