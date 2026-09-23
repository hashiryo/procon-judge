#pragma once
// 下敷き: 1 積ずつスカラで回す版 (現行の _shared/gf2-64/mul.hpp をそのまま使う)。
// 2 並列にした分がどれだけ効いているかを、この行との差で見る。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
struct GF2_64Op {
 GNU_TARGET("pclmul") static vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
  const size_t n= as.size();
  vector<u64> ans(n);
  for(size_t i= 0; i < n; ++i) ans[i]= gf2_64_pclmul::mul(as[i], bs[i]);
  return ans;
 }
};
