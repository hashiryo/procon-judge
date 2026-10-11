#pragma once
// 原始根の 1 回目 (核は _pr1.hpp)。p - 1 を NeoLibrary の factorize_pairs で分解し、小さい素数 g を Jacobi 記号と
// 4 bit の窓の Montgomery 累乗で試す。
#include "_pr1.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr1::primitive_root(p));
    return ans;
}
