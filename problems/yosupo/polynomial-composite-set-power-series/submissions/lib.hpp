#pragma once
// 自前ライブラリ (judge/lib/mylib) の sps::poly_comp<ModInt<998244353>> を呼ぶ。O(n^2 · 2^n + ?)。
// ModInt の列への写しと戻しは、構築と answer で行う (計測の外)。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/set_power_series.hpp"

struct Solver {
 using Mint = ModInt<998244353>;
 vector<Mint> P, ff, c;
 Solver(int M, int, const vector<u32>& f_in, const vector<u32>& b_in) : P(M), ff(b_in.size()) {
  for (int i = 0; i < M; ++i) P[i] = Mint(f_in[i]);
  for (size_t i = 0; i < b_in.size(); ++i) ff[i] = Mint(b_in[i]);
 }
 void run() { c = sps::poly_comp(P, ff); }
 vector<u32> answer() const {
  vector<u32> result(c.size());
  for (size_t i = 0; i < c.size(); ++i) result[i] = (u32)c[i].val();
  return result;
 }
};
