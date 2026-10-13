#pragma once
// 今の Library (mylib) の DiscreteLogarithm を呼ぶ (raw のときの lib.cpp を run の形に写したもの)。法は問ごとに
// ModInt_Runtime に設定する。
#include "../common.hpp"
#include "mylib/algebra/ModInt_Runtime.hpp"
#include "mylib/algebra/DiscreteLogarithm.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    using Mint = ModInt_Runtime<int>;
    auto f = [](Mint a, Mint b) { return a * b; };
    DiscreteLogarithm log(f, f, [](Mint x) { return x.val(); }, 1e9);
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) {
        Mint::set_mod(M);
        ans.push_back(log(X, 1, Y));
    }
    return ans;
}
