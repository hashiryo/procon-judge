#pragma once
// 原始根の 4 回目 (核は _pr4.hpp)。p - 1 を 4096 未満の奇素数で試し割りしてから、残りを factorize_pairs で分解する。
#include "_pr4.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr4::primitive_root<4096>(p));
    return ans;
}
