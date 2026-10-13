#pragma once
// 法が固定の離散対数の 3 回目 (核は _dlf3.hpp)。Pohlig-Hellman の baby-step を 4 本の鎖と 4 個ずつの籠の表にし、giant-step を的ごとに並べる。K1 = 10、SM = 200 で、√p より大きい合成数は区画ごとに素数の倍数として埋める。
#include "_dlf3.hpp"

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const dlf3::DlogFixed<10, 200, false, 8, true> D(p, g);
    vector<u32> ans;
    dlf3::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = D(qs[i]);
    return ans;
}
