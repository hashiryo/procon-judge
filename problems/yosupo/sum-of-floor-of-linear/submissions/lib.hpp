#pragma once
// 今の Library (mylib) の FloorSumTable を呼ぶ (raw のときの lib.cpp を run の形に写したもの)。
#include "../common.hpp"
#include "mylib/algebra/AllPurposeEuclid.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    using FST = FloorSumTable<long long, 0, 1>;
    FST::init();
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back((u64)FST::solve((long long)N - 1, A, B, M, 0, 1)[0][1]);
    return ans;
}
