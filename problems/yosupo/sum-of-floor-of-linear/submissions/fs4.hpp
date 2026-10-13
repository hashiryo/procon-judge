#pragma once
// 線形関数の床の和の 4 回目 (核は _fs4.hpp)。最初の段は a, b < m なので割り算 1 回だけにする。
#include "_fs4.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back(fs4::floor_sum(N, M, A, B));
    return ans;
}
