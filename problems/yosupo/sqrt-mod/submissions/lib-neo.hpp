#pragma once
// NeoLibrary の sqrt_mod を呼ぶ。中身は 4 回目 (_sq4.hpp) の sq4_t8nb と同じ手順で、P < 2^30 は 32 bit の Montgomery、
// それより大きい P は 64 bit の Montgomery の道を通る。lib.hpp は今の Library の mod_sqrt。
#include "../common.hpp"
#include "neo/number_theory/sqrt_mod.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(sqrt_mod(Y, P));
    return ans;
}
