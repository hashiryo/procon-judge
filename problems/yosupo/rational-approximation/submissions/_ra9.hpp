#pragma once
// 有理数近似の 9 回目。8 回目 (_ra8.hpp) の道の選び方を替える。先に 4 連を割り算で進めて商を見る代わりに、q_0 (整数部分)
// の次からは商の小さい道で進み、商 3 以上が出たらその段を割り算で済ませて、残りを割り算の道で進める。一様な分数は
// 商 3 以上が 4 割ほどの確率で出るので 2 項ほどで移り、商が 1 と 2 だけの分数は最後まで商の小さい道を通る。
// 移るときの分岐は 1 問に 1 回外れる。
#include "_ra8.hpp"

namespace ra9 {

using ra8::advance;

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

}  // namespace ra9
