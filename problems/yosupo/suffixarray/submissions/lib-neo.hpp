#pragma once
// NeoLibrary の suffix_array (neo/string/suffix_array.hpp、sais_lr13 と同じ中身) を呼ぶ。
// lib-suffix-array.hpp は今の Library (mylib) の SuffixArray。
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() { sa = suffix_array(s); }

  const vector<int> &answer() const { return sa; }
};
