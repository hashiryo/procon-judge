#pragma once
// NeoLibrary の floor_sum を呼ぶ。中身は 3 回目 (_fs3.hpp) の fs3_v0 と同じ手順 (割り算 3 回を互いに待たない互除法) で、
// AtCoder Library と同じ約束 (負の a、b も受ける) の入り口を通る。
#include "../common.hpp"
#include "neo/number_theory/floor_sum.hpp"

inline vector<u64> run(const vector<array<u32, 4>>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (auto [N, M, A, B] : qs) ans.push_back((u64)floor_sum(N, M, A, B));
    return ans;
}
