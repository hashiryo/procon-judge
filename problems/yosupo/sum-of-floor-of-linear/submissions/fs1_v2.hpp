#pragma once
// 線形関数の床の和の 1 回目 (核は _fs1.hpp)。割り算の形は V = 2。
#include "_fs1.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back(fs1::floor_sum<2>(N, M, A, B));
    return ans;
}
