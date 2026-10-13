#pragma once
// 線形関数の床の和の 2 回目 (核は _fs2.hpp)。互除法の 1 段の割り算 3 回を double の 1/m との掛け算にする。
#include "_fs2.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back(fs2::floor_sum(N, M, A, B));
    return ans;
}
