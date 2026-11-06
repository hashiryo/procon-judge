#pragma once
// 今の Library の SuffixArray と LCPArray。LCPArray(i, j) が接尾辞の順位の間の LCP 配列の最小を sparse table で引く。
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

struct Solver {
  SuffixArray<string> sa;
  LCPArray lcpa;
  explicit Solver(const string &s) : sa(s), lcpa(sa) {}
  int lcp(int i, int j) { return lcpa(i, j); }
};
