#pragma once
#include <numeric>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/counting/FactorialPrecalculation.hpp"
#include "mylib/number_theory/ArrayOnDivisors.hpp"

// 周期が d を割る並べ方の数を約数上の配列に載せ、倍数メビウス変換で周期がちょうど
// d のものに直す。周期 d の並びは回転で d 個ずつ同一視されるので、d を掛けて全体の
// 枚数で割る。
struct Solver {
  vector<i64> c;
  i64 ans = 0;

  explicit Solver(const vector<i64> &c) : c(c) {}

  void run() {
    using Mint = ModInt<1000000007>;
    using F = FactorialPrecalculation<Mint>;
    int tot = 0, g = 0;
    for (i64 v : c) tot += (int)v, g = std::gcd(g, (int)v);
    ArrayOnDivisors<int, Mint> a(g);
    for (auto &[d, v] : a) {
      v = F::fact(tot / d);
      for (i64 x : c) v *= F::finv((int)x / d);
    }
    a.multiple_mobius();
    Mint sum = 0;
    for (auto [d, v] : a) sum += v * d;
    sum /= tot;
    ans = sum.val();
  }

  i64 answer() const { return ans; }
};
