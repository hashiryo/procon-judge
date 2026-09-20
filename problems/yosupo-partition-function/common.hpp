#pragma once
// ライブラリを使う提出が共有するもの。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<998244353>;

inline vector<i64> from_mint(const vector<Mint> &a) {
  vector<i64> r(a.size());
  for (size_t i = 0; i < a.size(); ++i) r[i] = a[i].val();
  return r;
}
