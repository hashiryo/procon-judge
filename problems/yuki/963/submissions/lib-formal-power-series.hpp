#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/FormalPowerSeries.hpp"

// 形式的冪級数を f = integ((f^2 + 1) / 2) + 1 の不動点として定義し、必要な次数まで
// 遅延評価で係数を出す。式をそのまま書けるが、係数ごとに演算の木を辿る。
struct Solver {
  int n;
  i64 ans = 0;

  explicit Solver(int n) : n(n) {}

  void run() {
    using Mint = ModInt<1012924417>;
    using FPS = FormalPowerSeries<Mint>;
    FPS f;
    f.reset().set(integ((f * f + 1) / 2) + 1);
    Mint v = f[n] * 2;
    for (int i = n; i; i--) v *= i;
    ans = v.val();
  }

  i64 answer() const { return ans; }
};
