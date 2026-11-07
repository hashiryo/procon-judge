#pragma once
// NeoLibrary の find_all (neo/string/find_all.hpp) を呼ぶ提出。中身は、この問題で書き比べた simd_kmp5 を写したもの。
// 文字列の基本の道具を比べた記録は algo-notes の notes/string-basics.md にある。
#include "pj.hpp"
#include "neo/string/find_all.hpp"

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() { pos = find_all(t, p); }
  const vector<int> &answer() const { return pos; }
};
