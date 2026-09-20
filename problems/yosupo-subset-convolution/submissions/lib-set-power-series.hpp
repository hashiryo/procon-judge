#pragma once
#include "common.hpp"
#include "mylib/algebra/set_power_series.hpp"

// ランク付きゼータ変換で畳む専用の実装。O(2^N N^2)。
struct Solver {
  vector<Mint> a, b, c;

  Solver(int, const vector<i64> &a, const vector<i64> &b)
      : a(to_mint(a)), b(to_mint(b)) {}

  void run() { c = sps::convolve(a, b); }

  vector<i64> answer() const { return from_mint(c); }
};
