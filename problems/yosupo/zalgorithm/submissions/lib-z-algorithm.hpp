#pragma once
#include "pj.hpp"
#include "mylib/string/z_algorithm.hpp"

// Z 配列を直接作る O(N) の実装。この問題に対する素直な解。
struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() { z = z_algorithm(s); }

  const vector<int> &answer() const { return z; }
};
