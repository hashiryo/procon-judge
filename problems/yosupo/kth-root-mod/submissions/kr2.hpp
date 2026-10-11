#pragma once
// k 乗根 mod p の 2 回目 (核は _kr2.hpp)。1 回目の離散対数の表をスタックのハッシュ表にし、最後の桁の逆元を作らない。
#include "_kr2.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr2::kth_root(K, Y, P));
    return ans;
}
