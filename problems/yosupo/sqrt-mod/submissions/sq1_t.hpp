#pragma once
// 平方根 mod p の 1 回目 (核は _sq1.hpp)。P ≡ 1 (mod 8) で P - 1 の 2 の冪が 30 以下なら Tonelli-Shanks、それより大きければ Cipolla。
#include "_sq1.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(sq1::sqrt_mod<30>(Y, P));
    return ans;
}
