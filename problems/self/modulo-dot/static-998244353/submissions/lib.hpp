#pragma once
// 今の Library の ModInt<998244353> をそのまま使い、積を 1 つずつ ModInt の掛け算と足し算で足す (中身は MP_Mo32)。
// mylib/algebra/ModInt.hpp が変わると測り直される。
#include "_shared/modulo-test/_common.hpp"
#include "mylib/algebra/ModInt.hpp"
struct MP {
 using M= ModInt<998244353>;
 constexpr MP(u32) {}
 constexpr inline M set(u32 n) const { return M(n); }
 constexpr inline u32 get(M v) const { return v.val(); }
 inline M dot(const M* a, const M* b, size_t n) const {
  M s= 0;
  for (size_t k= 0; k < n; ++k) s+= a[k] * b[k];
  return s;
 }
};
