#pragma once
// NeoLibrary の stern_brocot.hpp の sb_approx を呼ぶ。中身は 11 回目 (ra11.hpp) と同じ手順。
#include "../common.hpp"
#include "neo/number_theory/stern_brocot.hpp"

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) {
        const auto r = sb_approx(q.n, q.x, q.y);
        out[0] = r[0], out[1] = r[1], out[2] = r[2], out[3] = r[3];
        out += 4;
    }
}
