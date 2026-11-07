#pragma once
// NeoLibrary の prefix_function (neo/string/prefix_function.hpp) を呼ぶ提出。中身は、この問題で書き比べた kmp_tab2 を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/prefix_function.hpp"

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() { pi = prefix_function(s); }
  const vector<int> &answer() const { return pi; }
};
