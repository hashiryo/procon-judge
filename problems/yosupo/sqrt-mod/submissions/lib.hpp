#pragma once
// 今の Library (mylib) の mod_sqrt を呼ぶ (raw のときの lib.cpp を run の形に写したもの)。
#include "../common.hpp"
#include "mylib/number_theory/mod_sqrt.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(mod_sqrt(Y, P));
    return ans;
}
