#pragma once
// 平方根 mod p の 2 回目 (核は _sq2.hpp)。P ≡ 1 (mod 8) で P - 1 の 2 の冪が 0 以下なら Tonelli-Shanks、それより大きければ Cipolla。
#include "_sq2.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(sq2::sqrt_mod<0>(Y, P));
    return ans;
}
