#pragma once
// 離散対数の 3 回目 (核は _dl3.hpp)。B を √r の 2^-0 倍にし、表を B の 4 倍以上にとる。
#include "_dl3.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) ans.push_back(dl3::discrete_log<4, 0>(X, Y, M));
    return ans;
}
