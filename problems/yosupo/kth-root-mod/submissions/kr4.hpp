#pragma once
// k 乗根 mod p の 4 回目 (核は _kr4.hpp)。逆元を NeoLibrary の inv_mod に替え、小さい g の試し割りを素数の表と逆元の掛け算にする。
#include "_kr4.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr4::kth_root(K, Y, P));
    return ans;
}
