#pragma once
#include <numeric>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/counting/FactorialPrecalculation.hpp"
#include "mylib/number_theory/ArrayOnDivisors.hpp"

// バーンサイドの補題。約数 d ごとに、周期 d で並べる多項係数に phi(d) を掛けて
// 足し、全体の枚数で割る。約数上の配列に Euler の phi を載せて使う。
struct Solver {
  vector<i64> c;
  i64 ans = 0;

  explicit Solver(const vector<i64> &c) : c(c) {}

  void run() {
    using Mint = ModInt<1000000007>;
    using F = FactorialPrecalculation<Mint>;
    int tot = 0, g = 0;
    for (i64 v : c) tot += (int)v, g = std::gcd(g, (int)v);
    Mint sum = 0;
    ArrayOnDivisors<int, Mint> a(g);
    a.set_totient();
    for (auto [d, phi] : a) {
      Mint tmp = F::fact(tot / d);
      for (i64 v : c) tmp *= F::finv((int)v / d);
      tmp *= phi;
      sum += tmp;
    }
    sum /= tot;
    ans = sum.val();
  }

  i64 answer() const { return ans; }
};
