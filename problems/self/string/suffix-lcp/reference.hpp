#pragma once
// 参照実装。接尾辞配列と LCP 配列の sparse table (_shared/string/_ref_lcp.hpp)。
#include "_shared/string/_ref_lcp.hpp"

struct Solver {
  string_ref::SuffixLCP t;
  explicit Solver(const string &s) : t(s) {}
  int lcp(int i, int j) { return t.lcp(i, j); }
};
