#pragma once
// 出力の配列を確保して 0 で埋めるだけを run() で測る診断用の提出。Z 配列は answer() で z_w8b と同じ計算をするので、出力は正しいまま。
// 確保した領域に初めて触る代価 (4 KB のページごとにメモリを割り当てて 0 で埋める) が、Z 配列の時間のどれだけを占めるかを見る。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_z_w8.hpp"

struct Solver {
  string s;
  mutable vector<int> z;
  mutable bool done = false;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    z.assign(s.size(), 0);
    asm volatile("" : : "r"(z.data()) : "memory");
  }

  const vector<int> &answer() const {
    if (!done) z_w8::z_array<true>(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), z.data()), done = true;
    return z;
  }
};
