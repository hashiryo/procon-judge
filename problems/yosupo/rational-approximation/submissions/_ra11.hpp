#pragma once
// 有理数近似の 11 回目。9 回目 (_ra9.hpp) と同じ手順で、答えを書くところの分岐をなくす。x < y で入れ替えたかを表す sw
// による書き分けは、一様な分数では半分の確率で外れる分岐になっていた。書く位置を sw からの添字の計算にする。
#include "../common.hpp"

namespace ra11 {

// 連 t 歩を進める (8 回目の advance と同じ)。答えを書くところで sw の分岐を使わない (一様な分数では x < y が半分の
// 確率で外れる)。下の端 (lo0/lo1) と上の端 (hi0/hi1) を、sw なら分子と分母、下と上を入れ替えた位置に書く
template <bool low>
[[gnu::always_inline]] inline bool advance(u32 q, u32 r, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out) {
    const u64 t = q - (r == 0);
    if (mx + (t + 1) * fx <= n) {
        mx += t * fx, my += t * fy;
        if (r != 0) return false;
        const u32 X = (u32)(mx + fx), Y = (u32)(my + fy), m = 0u - sw, A = X ^ ((X ^ Y) & m), B = X ^ Y ^ A;
        out[0] = out[2] = A, out[1] = out[3] = B;
        return true;
    }
    const u64 c = (n - mx) / fx;
    mx += c * fx, my += c * fy;
    const u32 lo0 = (u32)(low ? mx : fx), lo1 = (u32)(low ? my : fy), hi0 = (u32)(low ? fx : mx), hi1 = (u32)(low ? fy : my);
    out[3 * sw] = lo0, out[1 + sw] = lo1, out[2 - sw] = hi0, out[3 - 3 * sw] = hi1;
    return true;
}

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    const u32 sw = x < y;
    u32 a = sw ? y : x, b = sw ? x : y;
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    {
        const u32 q = a / b, r = a % b;
        if (advance<true>(q, r, lx, ly, rx, ry, n, sw, out)) return;
        a = b, b = r;
    }
    // 商の小さい道。L の連から
    while (true) {
        {
            u32 d = a - b, ge;
#if defined(__x86_64__)
            asm("xorl %k1, %k1\n\tcmpl %k2, %k0\n\tsetae %b1\n\tcmovae %k3, %k0" : "+r"(d), "=&r"(ge) : "r"(b), "r"(d - b) : "cc");
#else
            ge = d >= b;
            d = ge ? d - b : d;
#endif
            if (d >= b) [[unlikely]] {
                asm volatile("");
                const u32 q = a / b, r = a % b;
                if (advance<false>(q, r, rx, ry, lx, ly, n, sw, out)) return;
                a = b, b = r;
                goto big_R;
            }
            if (d == 0) [[unlikely]] {
                advance<false>(1 + ge, d, rx, ry, lx, ly, n, sw, out);
                return;
            }
            const u64 m = 0ull - ge;
            const u64 nx = rx + lx + (lx & m);
            if (nx + lx > n) [[unlikely]] {
                advance<false>(1 + ge, d, rx, ry, lx, ly, n, sw, out);
                return;
            }
            rx = nx, ry += ly + (ly & m);
            a = b, b = d;
        }
        {
            u32 d = a - b, ge;
#if defined(__x86_64__)
            asm("xorl %k1, %k1\n\tcmpl %k2, %k0\n\tsetae %b1\n\tcmovae %k3, %k0" : "+r"(d), "=&r"(ge) : "r"(b), "r"(d - b) : "cc");
#else
            ge = d >= b;
            d = ge ? d - b : d;
#endif
            if (d >= b) [[unlikely]] {
                asm volatile("");
                const u32 q = a / b, r = a % b;
                if (advance<true>(q, r, lx, ly, rx, ry, n, sw, out)) return;
                a = b, b = r;
                goto big_L;
            }
            if (d == 0) [[unlikely]] {
                advance<true>(1 + ge, d, lx, ly, rx, ry, n, sw, out);
                return;
            }
            const u64 m = 0ull - ge;
            const u64 nx = lx + rx + (rx & m);
            if (nx + rx > n) [[unlikely]] {
                advance<true>(1 + ge, d, lx, ly, rx, ry, n, sw, out);
                return;
            }
            lx = nx, ly += ry + (ry & m);
            a = b, b = d;
        }
    }
    // 割り算の道
big_R:
    while (true) {
        {
            const u32 q = a / b, r = a % b;
            if (advance<true>(q, r, lx, ly, rx, ry, n, sw, out)) return;
            a = b, b = r;
        }
    big_L:
        {
            const u32 q = a / b, r = a % b;
            if (advance<false>(q, r, rx, ry, lx, ly, n, sw, out)) return;
            a = b, b = r;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra11
