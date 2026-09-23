#pragma once
#include "common.hpp"
#include "mylib/fft/fps_exp.hpp"

// exp と log に落として M 乗を作る O(N log N)。係数を配列でまとめて持つオフラインの実装。
struct Solver {
  vector<Mint> a, b;
  i64 m;

  Solver(const vector<i64> &src, i64 m) : a(to_mint(src)), m(m) {}

  void run() { b = pow(a, m); }

  vector<i64> answer() const { return from_mint(b); }
};
