#pragma once
// k 乗根 mod p の 6 回目 (核は _kr6.hpp)。最後の指数を先に掛け、2 乗の列を共有して累乗を減らし、baby-step giant-step を正の向きにする。
#include "_kr6.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr6::kth_root(K, Y, P));
    return ans;
}
