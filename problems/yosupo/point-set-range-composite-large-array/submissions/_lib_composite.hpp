#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

// 今の Library の木に載せる 1 次関数の合成のモノイド。point-set-range-composite の lib と同じ形 (pair<Mint, Mint>)。
using Mint = ModInt<998244353>;
struct RcompositeQ {
  using T = pair<Mint, Mint>;  // x -> first x + second
  static T ti() { return make_pair(Mint(1), Mint(0)); }
  static T op(const T &l, const T &r) { return make_pair(r.first * l.first, r.first * l.second + r.second); }
};
