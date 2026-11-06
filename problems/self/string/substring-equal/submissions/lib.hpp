#pragma once
// 今の Library の RollingHash。法 998244353 の 2 つの基数の組 (Pointwise) で持つ (aoj-ALDS1_14_B などの lib と同じ形)。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "mylib/string/RollingHash.hpp"

struct Solver {
  using Mint = ModInt<998244353>;
  using K = Pointwise<Mint, Mint>;
  using RH = RollingHash<K>;
  RH rh;
  explicit Solver(const string &s) {
    RH::init({rng(), rng()});
    rh = RH(s);
  }
  bool equal(int a, int b, int len) { return rh.sub(a, len) == rh.sub(b, len); }
};
