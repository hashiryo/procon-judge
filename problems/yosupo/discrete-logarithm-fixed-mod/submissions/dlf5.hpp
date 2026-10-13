#pragma once
// 法が固定の離散対数の 5 回目 (核は _dlf5.hpp)。4 回目の表を作る側で、定数と表の場所を局所変数に写す (gcc が輪の中で
// メンバを読み直さないように)。K1 = 10、SM = 200。
#include "_dlf5.hpp"

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const dlf5::DlogFixed<10, 200, false, 8, false, true, true> D(p, g);
    const auto V = D.view();
    vector<u32> ans;
    dlf5::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = V(qs[i]);
    return ans;
}
