#pragma once
// 法が固定の離散対数の 4 回目 (核は _dlf4.hpp)。p mod i の段で 2 か 3 で割り切れる i を log 2 + log(i / 2) か
// log 3 + log(i / 3) で埋め、引く側は定数だけを持つ値の構造体で引く。K1 = 10、SM = 200。
#include "_dlf4.hpp"

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const dlf4::DlogFixed<10, 200, false, 8, false, true, true> D(p, g);
    const auto V = D.view();
    vector<u32> ans;
    dlf4::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = V(qs[i]);
    return ans;
}
