#pragma once
#include "common.hpp"
#include "neo/data_structure/Sequence.hpp"

// NeoLibrary の neo/data_structure/Sequence.hpp (塊の形)。
struct Solver {
  Sequence<int> t;
  explicit Solver(const vector<i64> &a) : t(to_int(a)) {}
  // 3 つに割って、真ん中をさらに 2 つに割り、順を入れ替えて繋ぎ直す。
  void rotate(int b, int m, int e) {
    auto [l, c, r] = t.split3(b, e);
    auto [cl, cr] = c.split(m - b);
    t = l + cr + cl + r;
  }
  vector<i64> dump() { return to_i64(t.dump()); }
};
