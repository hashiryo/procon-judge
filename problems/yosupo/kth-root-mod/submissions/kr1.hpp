#pragma once
// k 乗根 mod p の 1 回目 (核は _kr1.hpp)。g = gcd(K, P - 1) の素因数ごとに、r の Sylow 部分群とそれ以外に分けて根を取る。
#include "_kr1.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kr1::kth_root(K, Y, P));
    return ans;
}
