#pragma once
#include "common.hpp"
#include "mylib/fft/FormalPowerSeries.hpp"

// 係数を 1 つずつ遅延で求める relaxed convolution の実装。値を取り出すまで
// 計算が始まらないので、計測区間で N 個の読み出しまでやる。読み出しを外に出すと
// 何も測らないことになる。
struct Solver {
  using FPS = FormalPowerSeries<Mint>;

  int n;
  FPS f;
  vector<Mint> b;

  explicit Solver(const vector<i64> &src) : n((int)src.size()), f(to_mint(src)) {}

  void run() {
    FPS r = exp(f);
    b.resize(n);
    for (int i = 0; i < n; ++i) b[i] = r[i];
  }

  vector<i64> answer() const { return from_mint(b); }
};
