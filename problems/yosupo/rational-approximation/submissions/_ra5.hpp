#pragma once
// 有理数近似の 5 回目。1 回目 (_ra1.hpp) の互除法で、商が 1 の段は割り算を省く (a - b < b なら商 1、余り a - b)。
// 分岐で書くので、商 1 が続く分数 (fibonacci、small_run_length) では予測が当たり続けて 1 項数サイクルで進む。
// 一様な分数では商 1 が 4 割ほどで、分岐は外れやすい。2 段目からは a > b なので、商 0 は 1 段目にしか出ない。
#include "../common.hpp"

namespace ra5 {

[[gnu::always_inline]] inline void divmod(u32 a, u32 b, u32& q, u32& r) {
    const u32 d = a - b;
    if (d < b) q = 1, r = d;
    else q = a / b, r = a % b;
}

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 a = x, b = y;
    bool first = true;
    while (true) {
        // R の連
        {
            u32 q, r;
            if (first) q = a / b, r = a % b, first = false;
            else divmod(a, b, q, r);
            const u64 t = q - (r == 0);
            if (lx + (t + 1) * rx <= n && ly + (t + 1) * ry <= n) {
                lx += t * rx, ly += t * ry;
                if (r == 0) {
                    out[0] = out[2] = (u32)(lx + rx), out[1] = out[3] = (u32)(ly + ry);
                    return;
                }
            } else {
                u64 c = (n - lx) / rx;
                if (ry) c = std::min(c, (n - ly) / ry);
                lx += c * rx, ly += c * ry;
                out[0] = (u32)lx, out[1] = (u32)ly, out[2] = (u32)rx, out[3] = (u32)ry;
                return;
            }
            a = b, b = r;
        }
        // L の連
        {
            u32 q, r;
            divmod(a, b, q, r);
            const u64 t = q - (r == 0);
            if (rx + (t + 1) * lx <= n && ry + (t + 1) * ly <= n) {
                rx += t * lx, ry += t * ly;
                if (r == 0) {
                    out[0] = out[2] = (u32)(lx + rx), out[1] = out[3] = (u32)(ly + ry);
                    return;
                }
            } else {
                u64 c = (n - ry) / ly;
                if (lx) c = std::min(c, (n - rx) / lx);
                rx += c * lx, ry += c * ly;
                out[0] = (u32)lx, out[1] = (u32)ly, out[2] = (u32)rx, out[3] = (u32)ry;
                return;
            }
            a = b, b = r;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra5
