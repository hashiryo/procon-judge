#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/sequences.hpp"

// 交代順列の数を直接生成する関数。母関数 sec + tan を計算して階乗を掛ける、
// 決まった手順で出す。
struct Solver {
  int n;
  i64 ans = 0;

  explicit Solver(int n) : n(n) {}

  void run() {
    using Mint = ModInt<1012924417>;
    ans = (alternating_permutation<Mint>(n)[n] * 2).val();
  }

  i64 answer() const { return ans; }
};
