#pragma once
// 2n - 1 個の中心を 1 本のループで回す Manacher (中身は _manacher.hpp)。伸ばすところは 1 文字目を比べてから 8 バイトずつ比べる。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_manacher.hpp"

struct Solver {
  string s;
  vector<int> len;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    len.assign(n > 0 ? 2 * n - 1 : 0, 0);
    manacher::palindromes<true>(reinterpret_cast<const unsigned char *>(s.data()), n, len.data());
  }

  const vector<int> &answer() const { return len; }
};
