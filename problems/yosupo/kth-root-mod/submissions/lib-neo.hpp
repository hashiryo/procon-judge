#pragma once
// NeoLibrary の kth_root_mod を呼ぶ。中身は 6 回目 (_kr6.hpp) と同じ手順で、P < 2^30 は 32 bit の Montgomery、それより
// 大きい P は 64 bit の Montgomery の道を通る。lib.hpp は今の Library の mod_kth_root。
#include "../common.hpp"
#include "neo/number_theory/kth_root_mod.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [K, Y, P] : qs) ans.push_back(kth_root_mod(K, Y, P));
    return ans;
}
