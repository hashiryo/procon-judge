#pragma once
// 線形関数の床の和の 3 回目 (核は _fs3.hpp)。互除法の 1 段の割り算 3 回を互いに待たない形に並べる。V = 0。
#include "_fs3.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back(fs3::floor_sum<0>(N, M, A, B));
    return ans;
}
