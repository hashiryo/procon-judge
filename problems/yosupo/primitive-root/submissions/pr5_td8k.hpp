#pragma once
// 原始根の 5 回目 (核は _pr5.hpp)。4 回目の 8192 の版で、二分木の 2 つの累乗を同じ表で並べて回す。
#include "_pr5.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr5::primitive_root<8192>(p));
    return ans;
}
