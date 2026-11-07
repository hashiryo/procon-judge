#pragma once
// NeoLibrary の SuffixLCP (neo/string/SuffixLCP.hpp) を呼ぶ提出。中身は、この問題で書き比べた sa_rmq を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/SuffixLCP.hpp"

struct Solver {
  SuffixLCP L;
  explicit Solver(const string &s) : L(s) {}
  int lcp(int i, int j) { return L.lcp(i, j); }
};
