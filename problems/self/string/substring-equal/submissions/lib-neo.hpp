#pragma once
// NeoLibrary の RollingHash (neo/string/RollingHash.hpp) を呼ぶ提出。中身は、この問題で書き比べた rh61_skew を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/RollingHash.hpp"

struct Solver {
  RollingHash rh;
  explicit Solver(const string &s) : rh(s) {}
  bool equal(int a, int b, int len) { return rh.equal(a, b, len); }
};
