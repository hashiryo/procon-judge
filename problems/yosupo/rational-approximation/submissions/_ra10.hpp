#pragma once
// 有理数近似の 10 回目。9 回目 (_ra9.hpp) の商の小さい道を商 3 まで広げる。d = a - b、d - b、d - 2b を作り、
// d ≥ b と d - b ≥ b の 2 つの比べで、2 段の cmov で余りを選ぶ (商は 1 + 2 つの比べ)。鎖は引き算、比べ、cmov 2 段の
// 5 サイクルほど。商 4 以上 (選んだ余りが b 以上) で割り算の道に移る。small_run_length_02 (商が 1、2、3 で均等) を
// 商の小さい道のまま進めるための版。
#include "_ra8.hpp"

namespace ra10 {

using ra8::advance;

// a > b のとき、商が 3 以下なら余り d と商 - 1 を ge に返す。商 4 以上なら d ≥ b になる。a ≥ 2b と a ≥ 3b を比べ、
// a - b、a - 2b、a - 3b を cmov で選ぶ (2b と 3b は b から 1 サイクルで作れるので、鎖は 4 サイクルほど)。3b < 2^32 が前提
[[gnu::always_inline]] inline void sel3(u32 a, u32 b, u32& d, u32& ge) {
    const u32 b2 = b + b, b3 = 3 * b;
#if defined(__x86_64__)
    u32 r = a - b, g1, g2;
    asm("xorl %k1, %k1\n\t"
        "xorl %k2, %k2\n\t"
        "cmpl %k4, %k3\n\t"
        "setae %b1\n\t"
        "cmovae %k5, %k0\n\t"
        "cmpl %k6, %k3\n\t"
        "setae %b2\n\t"
        "cmovae %k7, %k0"
        : "+r"(r), "=&r"(g1), "=&r"(g2)
        : "r"(a), "r"(b2), "r"(a - b2), "r"(b3), "r"(a - b3)
        : "cc");
    d = r, ge = g1 + g2;
#else
    const u32 g1 = a >= b2, g2 = a >= b3;
    d = g2 ? a - b3 : g1 ? a - b2 : a - b;
    ge = g1 + g2;
#endif
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
            u32 d, ge;
            sel3(a, b, d, ge);
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
            const u64 t = 1 + ge;
            const u64 nx = rx + t * lx;
            if (nx + lx > n) [[unlikely]] {
                advance<false>(1 + ge, d, rx, ry, lx, ly, n, sw, out);
                return;
            }
            rx = nx, ry += t * ly;
            a = b, b = d;
        }
        {
            u32 d, ge;
            sel3(a, b, d, ge);
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
            const u64 t = 1 + ge;
            const u64 nx = lx + t * rx;
            if (nx + rx > n) [[unlikely]] {
                advance<true>(1 + ge, d, lx, ly, rx, ry, n, sw, out);
                return;
            }
            lx = nx, ly += t * ry;
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

}  // namespace ra10
