#pragma once
// NeoLibrary の lyndon_factorization (neo/string/lyndon_factorization.hpp) を呼ぶ提出。中身は、この問題で書き比べた duval_skip を写したもの。
// Lyndon 分解の実装を比べた記録は algo-notes の notes/lyndon.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/lyndon_factorization.hpp"

struct Solver {
  string s;
  vector<int> cut;

  explicit Solver(const string &s) : s(s) {}

  void run() { cut = lyndon_factorization(s); }

  const vector<int> &answer() const { return cut; }
};
