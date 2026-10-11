#pragma once
// NeoLibrary の z_algorithm (neo/string/z_algorithm.hpp) を呼ぶ提出。中身は、この問題で書き比べた z_skip を写したもの。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/z_algorithm.hpp"

struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() { z = z_algorithm(s); }

  const vector<int> &answer() const { return z; }
};
