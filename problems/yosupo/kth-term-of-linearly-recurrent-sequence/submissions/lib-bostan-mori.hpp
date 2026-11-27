#pragma once
// 今の Library の Bostan–Mori (linear_recurrence) を、base の Solver の形にする。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/bostan_mori.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 std::vector<Mint> a, c;
 u64 k;
 i64 out= 0;

 Solver(const std::vector<i64>& aa, const std::vector<i64>& cc, u64 kk): a(aa.size()), c(cc.size()), k(kk) {
  for (int i= 0; i < int(aa.size()); ++i) a[i]= Mint(aa[i]);
  for (int i= 0; i < int(cc.size()); ++i) c[i]= Mint(cc[i]);
 }

 void run() { out= linear_recurrence(c, a, k).val(); }

 i64 value() const { return out; }
};
