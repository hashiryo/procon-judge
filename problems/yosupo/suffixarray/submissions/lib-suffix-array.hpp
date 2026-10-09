#pragma once
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

// Library の SuffixArray (SA-IS)。文字を int の列に直して番兵を足し、SA-IS で再帰する。raw のハーネスだったころの lib.cpp を
// ハーネスの形に合わせたもの。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    SuffixArray<string> x(s);
    sa = std::move(x.sa);
  }

  const vector<int> &answer() const { return sa; }
};
