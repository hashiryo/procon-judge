#pragma once
// 今の Library (mylib) の mod_kth_root を呼ぶ (raw のときの lib.cpp を run の形に写したもの)。
#include "../common.hpp"
#include "mylib/number_theory/mod_kth_root.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(mod_kth_root(Y, K, P));
    return ans;
}
