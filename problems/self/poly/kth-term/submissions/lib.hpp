#pragma once
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/bostan_mori.hpp"
inline u32 run(const vector<u32>& a, const vector<u32>& c, u64 k) {
 using Mint= ModInt<998244353>;
 vector<Mint> A(a.begin(), a.end()), C(c.begin(), c.end());
 return linear_recurrence<Mint>(C, A, k).val();
}
