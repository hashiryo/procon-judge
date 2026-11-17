#pragma once
// 自前ライブラリ (judge/lib/mylib) の sps::exp<ModInt<998244353>> を呼ぶ。O(n^2 · 2^n) の subset exp。
// ModInt の列への写しと戻しは、構築と answer で行う (計測の外)。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/set_power_series.hpp"

struct Solver {
 using Mint = ModInt<998244353>;
 vector<Mint> f, c;
 Solver(int, const vector<u32>& b_in) : f(b_in.size()) {
  for (size_t i = 0; i < b_in.size(); ++i) f[i] = Mint(b_in[i]);
 }
 void run() { c = sps::exp(f); }
 vector<u32> answer() const {
  vector<u32> result(c.size());
  for (size_t i = 0; i < c.size(); ++i) result[i] = (u32)c[i].val();
  return result;
 }
};
