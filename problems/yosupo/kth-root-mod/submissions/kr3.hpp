#pragma once
// k 乗根 mod p の 3 回目 (核は _kr3.hpp)。累乗を分岐なしの右から左の 2 進にし、小さい g は試し割りで分解する。
#include "_kr3.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr3::kth_root(K, Y, P));
    return ans;
}
