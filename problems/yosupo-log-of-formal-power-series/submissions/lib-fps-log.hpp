#pragma once
#include "common.hpp"
#include "mylib/fft/fps_exp.hpp"

// 微分して割って積分する O(N log N)。係数を配列でまとめて持つオフラインの実装。
struct Solver {
  vector<Mint> a, b;

  explicit Solver(const vector<i64> &src) : a(to_mint(src)) {}

  void run() { b = log(a); }

  vector<i64> answer() const { return from_mint(b); }
};
