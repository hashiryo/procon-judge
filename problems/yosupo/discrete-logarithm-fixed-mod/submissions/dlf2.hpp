#pragma once
// 法が固定の離散対数の 2 回目 (核は _dlf2.hpp)。2 段目の歩みを c^{-1} (c = g^K) の掛け算にする。K1 = 25、SM = 100 は 1 回目と同じ。
#include "_dlf2.hpp"

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const dlf2::DlogFixed<25, 100> D(p, g);
    vector<u32> ans;
    dlf2::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = D(qs[i]);
    return ans;
}
