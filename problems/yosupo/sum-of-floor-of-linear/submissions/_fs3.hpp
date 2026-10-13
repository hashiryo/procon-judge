#pragma once
// 線形関数の床の和の 3 回目。互除法の 1 段の割り算 3 回を、互いに待たない形に並べ替える。1 回目は a mod m と b mod m を
// 作ってから y = (a mod m) n + (b mod m) を m で割っていたので、割り算 2 回ぶんが 1 段の依存になっていた。ここでは
//   Q = ⌊(a n + b) / m⌋、r = (a n + b) mod m (a < 2^32、n < 2^30 なので a n + b < 2^62)
// を先に求め、次の n は Q - ⌊a/m⌋ n - ⌊b/m⌋、次の b は r とする (余りは同じ)。3 回の割り算は互いに待たない。V は
//   0: ⌊a/m⌋ と ⌊b/m⌋ も 32 bit の割り算
//   1: ⌊a/m⌋ と ⌊b/m⌋ は double の 1/m との掛け算 ((x + 1/2) (1/m) の切り捨て)。1/m は 1 段前に割り始める
#include "../common.hpp"

namespace fs3 {

template <int V>
inline u64 floor_sum(u32 n, u32 m, u32 a, u32 b) {
    u64 ans = 0;
    double im = V == 1 ? 1.0 / m : 0;
    while (true) {
        const u64 y = (u64)a * n + b;
        if (y < m) break;  // a、b を割らなくても、y < m なら和は 0 (a n + b < m なら ⌊(a i + b)/m⌋ = 0)
        const u64 Q = y / m;
        const u32 r = (u32)(y % m);
        u32 qa, qb;
        if constexpr (V == 1) qa = (u32)(((double)a + 0.5) * im), qb = (u32)(((double)b + 0.5) * im);
        else qa = a / m, qb = b / m;
        ans += (u64)n * (n - 1) / 2 * qa + (u64)n * qb;
        const u32 am = a - qa * m;  // a mod m
        const u64 nn = Q - (u64)qa * n - qb;
        if (nn == 0) break;  // (a mod m) n + (b mod m) < m
        if constexpr (V == 1) im = 1.0 / am;
        n = (u32)nn, b = r, a = m, m = am;
    }
    return ans;
}

}  // namespace fs3
