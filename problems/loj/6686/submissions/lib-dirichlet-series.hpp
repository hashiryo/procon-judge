#pragma once
#include "common.hpp"
#include "mylib/number_theory/DirichletSeries.hpp"

// Dirichlet 級数。gcd の和を約数関数の形に開いて、恒等関数と 2 乗の級数の積と商の
// 係数の和で出す。O(N^(2/3) log^(1/3) N)。
struct Solver {
  u128 n;
  i64 ans = 0;

  explicit Solver(u128 n) : n(n) {}

  void run() {
    Split s = split(n);
    const uint64_t r = s.r;
    Mint sum = s.tail;
    auto zeta = get_1<Mint>(r), id = get_Id<Mint>(r), id2 = get_Id2<Mint>(r);
    sum += (id2.square() / id).sum() * 3;
    sum += (id.square() / zeta).sum() * 3;
    sum += id.sum();
    ans = sum.val();
  }

  i64 answer() const { return ans; }
};
