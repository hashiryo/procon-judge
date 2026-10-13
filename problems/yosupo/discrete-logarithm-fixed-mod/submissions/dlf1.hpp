#pragma once
// 法が固定の離散対数の 1 回目 (核は _dlf1.hpp)。index_calculus.hpp と同じ手順で、区画の割り算を掛け算に、p mod i の段を
// 割り算なしに、剰余の掛け算を Montgomery にし、表を huge page に置く。
#include "_dlf1.hpp"

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const dlf1::DlogFixed D(p, g);
    vector<u32> ans;
    dlf1::reserve_huge(ans, qs.size());
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = D(qs[i]);
    return ans;
}
