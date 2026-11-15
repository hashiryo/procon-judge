#pragma once
#include "pj.hpp"
#include "_fastset_hp.hpp"

// 確かめの提出。lib-neo と同じく FastSet に値の範囲 [0, 2^30) を全部取るが、配列を 2 MB 境界に置いて huge page を頼む写し
// (_fastset_hp.hpp) を使う。lib-neo が bit64 より遅いのが、128 MB の葉を 4 KB のページで乱れた順に読む分かを見る。
struct Solver {
  FastSetHP s;
  Solver() : s(1 << 30) {}
  void insert(int x) { s.insert(x); }
  void erase(int x) { s.erase(x); }
  int xor_min(int x) { return s.xor_min(x) ^ x; }
};
