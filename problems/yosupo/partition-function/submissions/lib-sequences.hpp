#pragma once
#include "common.hpp"
#include "mylib/fft/sequences.hpp"

// 五角数定理で漸化式を回す O(N sqrt N) ではなく、専用の O(N log N)。
// 係数を配列でまとめて持つオフラインの実装。
struct Solver {
  int n;
  vector<Mint> b;

  explicit Solver(int n) : n(n) {}

  void run() { b = partition<Mint>(n); }

  vector<i64> answer() const { return from_mint(b); }
};
