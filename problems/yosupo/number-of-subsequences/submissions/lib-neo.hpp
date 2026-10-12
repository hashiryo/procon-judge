#pragma once
// NeoLibrary の distinct_subsequences (nos_adapt2 を写したもの) を呼ぶ提出。
#include <vector>
#include "pj.hpp"
#include "neo/string/distinct_subsequences.hpp"

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() { ans = distinct_subsequences(a); }

  unsigned answer() const { return ans; }
};
