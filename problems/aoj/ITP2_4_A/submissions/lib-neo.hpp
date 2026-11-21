#pragma once
#include "common.hpp"
#include "neo/base/ds/seq/Sequence.hpp"

// NeoLibrary の neo/base/ds/seq/Sequence.hpp (塊の形)。
struct Solver {
  Sequence<int, true> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  void reverse(int b, int e) { t.reverse(b, e); }
  vector<i64> dump() { return to_i64(t.dump()); }
};
