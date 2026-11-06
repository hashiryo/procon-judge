#pragma once
// NeoLibrary の z_algorithm (neo/string/z_algorithm.hpp) で Z 配列を求めてから、接頭辞関数に直す。位置 i から始まる長さ z[i] の
// 一致は、i + k - 1 で終わる長さ k の border を与える (1 <= k <= z[i])。i の小さい順に、まだ決まっていない位置だけを
// 後ろから埋める (先に決まった値の方が長い)。
#include "pj.hpp"
#include "neo/string/z_algorithm.hpp"

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const int n = (int)s.size();
    const vector<int> z = z_algorithm(s);
    pi.assign(n, 0);
    for (int i = 1; i < n; ++i)
      for (int k = z[i]; k > 0 && pi[i + k - 1] == 0; --k) pi[i + k - 1] = k;
  }
  const vector<int> &answer() const { return pi; }
};
