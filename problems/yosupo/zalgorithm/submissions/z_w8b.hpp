#pragma once
// z_w8 で、8 バイトを読む前に 1 文字目だけを比べ、外れたらそこで止める版 (中身は _z_w8.hpp)。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_z_w8.hpp"

struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    z.assign(s.size(), 0);
    z_w8::z_array<true>(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), z.data());
  }

  const vector<int> &answer() const { return z; }
};
