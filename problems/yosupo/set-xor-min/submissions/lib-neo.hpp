#pragma once
#include "pj.hpp"
#include "neo/base/ds/set/FastSet.hpp"

// NeoLibrary の FastSet (64 分木の bitset) に値の範囲 [0, 2^30) を全部取り、xor_min で x と xor して最小になる要素を引く。
// 葉の段だけで 128 MB あり、構築で 0 に埋める (構築は計測区間の外)。
struct Solver {
  FastSet s;
  Solver() : s(1 << 30) {}
  void insert(int x) { s.insert(x); }
  void erase(int x) { s.erase(x); }
  int xor_min(int x) { return s.xor_min(x) ^ x; }
};
