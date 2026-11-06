#pragma once
// 今の Library の SuffixArray::pattern_matching。p で始まる接尾辞の範囲を二分探索で求め、その幅を返す。
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

struct Solver {
  SuffixArray<string> sa;
  explicit Solver(const string &s) : sa(s) {}
  int count(const string &p) {
    const auto [l, r] = sa.pattern_matching(p);
    return r - l;
  }
};
