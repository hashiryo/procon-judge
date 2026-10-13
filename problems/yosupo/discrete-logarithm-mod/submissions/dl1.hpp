#pragma once
// 離散対数の 1 回目 (核は _dl1.hpp)。互いに素でない部分を外し、X の位数を φ(M) から求めて Pohlig-Hellman で解く。
#include "_dl1.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) ans.push_back(dl1::discrete_log(X, Y, M));
    return ans;
}
