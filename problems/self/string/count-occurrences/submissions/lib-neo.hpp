#pragma once
// NeoLibrary の FMIndex (neo/string/FMIndex.hpp) を呼ぶ提出。中身は、この問題で書き比べた fm_sa3 を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/FMIndex.hpp"

struct Solver {
  FMIndex fm;
  explicit Solver(const string &s) : fm(s) {}
  int count(const string &p) { return fm.count(p); }
};
