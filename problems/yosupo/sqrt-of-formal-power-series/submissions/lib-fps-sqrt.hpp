#pragma once
#include "common.hpp"
#include "mylib/fft/fps_sqrt.hpp"

// mylib の sqrt (Newton 法) で平方根を作る。無ければ空。
struct Solver {
  vector<Mint> a, b;

  explicit Solver(const vector<i64> &src) : a(to_mint(src)) {}

  void run() { b = sqrt(a); }

  vector<i64> answer() const { return b.empty() ? vector<i64>() : from_mint(b); }
};
