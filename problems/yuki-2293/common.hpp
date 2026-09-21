#pragma once
// 提出が共有するもの。
#include "pj.hpp"

constexpr i64 MOD = 998244353;

// 2^0 から 2^n までを mod で並べる。
inline vector<i64> pow2_table(int n) {
  vector<i64> pw(n + 1);
  pw[0] = 1;
  for (int i = 1; i <= n; ++i) pw[i] = pw[i - 1] * 2 % MOD;
  return pw;
}
