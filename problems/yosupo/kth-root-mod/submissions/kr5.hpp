#pragma once
// k 乗根 mod p の 5 回目 (核は _kr5.hpp)。位数 r の離散対数の baby-step giant-step を 4 本の鎖で並べて回す。
#include "_kr5.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr5::kth_root(K, Y, P));
    return ans;
}
