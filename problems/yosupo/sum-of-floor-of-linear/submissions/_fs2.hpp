#pragma once
// 線形関数の床の和の 2 回目。1 回目の互除法の 1 段で、割り算 3 回 (a / m、b / m、y / m) を、double の 1/m との掛け算に替える。
// - a / m と b / m は (x + 1/2) (1/m) の切り捨て。x < 2^32 なら誤差が 1/(2m) 未満なので割り算と一致する。
// - y = a n + b (a, b < m、n < 2^30) は 2^53 を超えうるので、q0 = ⌊y (1/m)⌋ を整数の余り r = y - q0 m で 1 だけ直す
//   (相対誤差は 3 × 2^-53、商は 2^30 未満なので q0 は真の商から ±1 以内)。
// 次の段の m は今の a mod m なので、次の 1/m の割り算は y を作るのと並べて始められる。
#include "../common.hpp"

namespace fs2 {

inline u64 floor_sum(u32 n, u32 m, u32 a, u32 b) {
    u64 ans = 0;
    double im = 1.0 / m;
    while (true) {
        if (a >= m) {
            const u32 q = (u32)(((double)a + 0.5) * im);
            ans += (u64)n * (n - 1) / 2 * q, a -= q * m;
        }
        if (b >= m) {
            const u32 q = (u32)(((double)b + 0.5) * im);
            ans += (u64)n * q, b -= q * m;
        }
        const u64 y = (u64)a * n + b;
        if (y < m) break;
        const double ia = 1.0 / a;  // 次の段の 1/m
        u64 q = (u64)((double)y * im);
        i64 r = (i64)(y - q * m);
        const i64 lt = r < 0, ge = r >= (i64)m;
        q = q - lt + ge, r = r + (i64)m * (lt - ge);
        n = (u32)q, b = (u32)r;
        std::swap(m, a);
        im = ia;
    }
    return ans;
}

}  // namespace fs2
