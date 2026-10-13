#pragma once
// 線形関数の床の和の 4 回目。3 回目 (V = 0) の最初の段で、a, b < m (問題の約束) なので ⌊a/m⌋ と ⌊b/m⌋ の割り算を省き、
// y = a n + b を m で割る 1 回だけにする。2 段目からは 3 回目と同じ (3 回の割り算を互いに待たない形)。
#include "../common.hpp"

namespace fs4 {

inline u64 floor_sum(u32 n, u32 m, u32 a, u32 b) {  // a, b < m
    {
        const u64 y = (u64)a * n + b;
        if (y < m) return 0;
        n = (u32)(y / m), b = (u32)(y % m);
        std::swap(m, a);  // a ≠ 0 (a = 0 なら y = b < m)
    }
    u64 ans = 0;
    while (true) {
        const u64 y = (u64)a * n + b;
        const u64 Q = y / m;
        const u32 r = (u32)(y % m), qa = a / m, qb = b / m;
        ans += (u64)n * (n - 1) / 2 * qa + (u64)n * qb;
        const u32 am = a - qa * m;
        const u64 nn = Q - (u64)qa * n - qb;
        if (nn == 0) break;
        n = (u32)nn, b = r, a = m, m = am;
    }
    return ans;
}

}  // namespace fs4
