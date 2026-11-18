#pragma once
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/sample_points_shift.hpp"
inline u32 run(const vector<u32>& y, u64 x) {
 using Mint= ModInt<998244353>;
 vector<Mint> Y(y.begin(), y.end());
 return sample_points_shift<Mint>(Y, Mint(x)).val();
}
