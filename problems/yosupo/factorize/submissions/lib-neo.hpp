#pragma once
// NeoLibrary の factorize (試し割り、平方根、2^48 未満は Pollard rho、それより大きければ ECM) を呼ぶ。
// lib.hpp は今の Library (mylib) の Factors。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"

inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(factorize(x));
 return ans;
}
