#pragma once
// NeoLibrary の primitive_root を呼ぶ。中身は 5 回目と 6 回目の形 (最初の平方非剰余の二分木、足りない q は乱数の候補で
// 満たして組み合わせる) で、p - 1 は NeoLibrary の factorize_pairs (2048 未満の試し割り、rho、ECM) で分解する。
// lib.hpp は今の Library の OrderFp。
#include "../common.hpp"
#include "neo/number_theory/primitive_root.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(primitive_root(p));
    return ans;
}
