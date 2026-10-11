#pragma once
// 相似変換で上 Hessenberg の形に直してから漸化式で特性多項式を求める (_hess.hpp)。行列は u64 で持つ。法は 998244353。
#include "_hess.hpp"
inline vector<u32> run(int n, const vector<vector<u32>>& M) { return hess::charpoly(n, M); }
