#pragma once
// 離散対数の 2 回目 (核は _dl2.hpp)。baby-step giant-step の表を 4 バイトの項にし、B の 16 倍以上の大きさにとる。
#include "_dl2.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) ans.push_back(dl2::discrete_log<16>(X, Y, M));
    return ans;
}
