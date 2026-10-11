#pragma once
// NeoLibrary の palindrome_lengths (neo/string/palindrome_lengths.hpp) を呼ぶ提出。中身は、この問題で書き比べた pal_skip を写したもの。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/palindrome_lengths.hpp"

struct Solver {
  string s;
  vector<int> len;

  explicit Solver(const string &s) : s(s) {}

  void run() { len = palindrome_lengths(s); }

  const vector<int> &answer() const { return len; }
};
