#pragma once
// NeoLibrary の DiscreteLogAction を、モノイド (Z/MZ, x) が集合 Z/MZ に掛け算で作用する形に使う一般版。
// 速い専用解法 (Pohlig-Hellman) がある問題でも、この口だけで解けることを見るための提出。
// 探索の上限は M (軌道の長さは M 以下なので、解があれば K < M)。
#include "../common.hpp"
#include "mylib/algebra/ModInt_Runtime.hpp"
#include "neo/algo/compute/DiscreteLogAction.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    using Mint = ModInt_Runtime<int>;
    auto act = [](Mint a, Mint b) { return a * b; };
    auto op = [](Mint a, Mint b) { return a * b; };
    auto hash = [](Mint x) { return (unsigned)x.val(); };
    DiscreteLogAction dlog(act, op, hash, 1e9);
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) {
        Mint::set_mod(M);
        ans.push_back(dlog(Mint(X), Mint(1), Mint(Y), (long long)M));
    }
    return ans;
}
