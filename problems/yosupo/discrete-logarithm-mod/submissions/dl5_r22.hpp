#pragma once
// 離散対数の 5 回目 (核は _dl5.hpp)。dl5::discrete_log<8, 1, (1ull << 22)> (表の大きさの倍率、B の縮め、rho 法に切り替える位数の下限)。
#include "_dl5.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) ans.push_back(dl5::discrete_log<8, 1, (1ull << 22)>(X, Y, M));
    return ans;
}
