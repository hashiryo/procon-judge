#pragma once
#include "common.hpp"
#include "mylib/fft/fps_exp.hpp"

// Newton 法で exp を作る O(N log N)。係数を配列でまとめて持つオフラインの実装。
struct Solver {
  vector<Mint> a, b;

  explicit Solver(const vector<i64> &src) : a(to_mint(src)) {}

  void run() { b = exp(a); }

  vector<i64> answer() const { return from_mint(b); }
};
