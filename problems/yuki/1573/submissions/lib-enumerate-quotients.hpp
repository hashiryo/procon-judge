#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/number_theory/enumerate_quotients.hpp"

// 商 floor(n / i) が同じ i の区間ごとにまとめて足す。区間は O(sqrt N) 個。
struct Solver {
  i64 n, m, ans = 0;

  Solver(i64 n, i64 m) : n(n), m(m) {}

  void run() {
    using Mint = ModInt<998244353>;
    Mint sum = 0;
    for (auto [q, l, r] : enumerate_quotients(n)) {
      const i64 h = std::min<i64>(m, r);
      if ((i64)l >= h) continue;
      sum += Mint((h * (h + 1) - (i64)l * (l + 1)) / 2) * ((i64)q * (q + 3) / 2);
    }
    ans = sum.val();
  }

  i64 answer() const { return ans; }
};
