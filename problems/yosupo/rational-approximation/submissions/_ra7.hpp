#pragma once
// 有理数近似の 7 回目。6 回目 (_ra6.hpp) の商の小さい道で、端の更新の掛け算もやめる。商が 1 か 2 なら、連の歩数 t も
// 1 か 2 (最後の連を除く) なので、動く端は m + f + (f & mask) (mask は t = 2 のとき全ビット 1) と足し算 2 回で進む。
// 連を全部進めた先が N を超えるかは、進めた端に f を足して比べる。最後の連と、N を超えた連と、商 3 以上の段は
// 6 回目の advance に任せる (どれも 1 問に高々 1 回か、商の小さい分数ではまれ)。
#include "_ra6.hpp"

namespace ra7 {

using ra6::advance;

// 商の小さい道の 1 段。答えが決まったら true
template <bool low>
[[gnu::always_inline]] inline bool step_small(u32& a, u32& b, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32* out) {
    if ((u64)a >= 3ull * b) [[unlikely]] {
        asm volatile("");
        const u32 q = a / b, r = a % b;
        if (advance<low>(q, r, mx, my, fx, fy, n, out)) return true;
        a = b, b = r;
        return false;
    }
    u32 d = a - b, ge;
#if defined(__x86_64__)
    asm("xorl %k1, %k1\n\t"
        "cmpl %k2, %k0\n\t"
        "setae %b1\n\t"
        "cmovae %k3, %k0"
        : "+r"(d), "=&r"(ge)
        : "r"(b), "r"(d - b)
        : "cc");
#else
    ge = d >= b;
    d = ge ? d - b : d;
#endif
    const u32 r = d;
    if (r == 0) [[unlikely]] return advance<low>(1 + ge, r, mx, my, fx, fy, n, out);
    const u64 m = 0ull - ge;
    const u64 nx = mx + fx + (fx & m), ny = my + fy + (fy & m);
    if (std::max(nx + fx, ny + fy) > n) [[unlikely]] return advance<low>(1 + ge, r, mx, my, fx, fy, n, out);
    mx = nx, my = ny;
    a = b, b = r;
    return false;
}

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 a = x, b = y, big = 0;
    for (int it = 0; it < 2; ++it) {
        u32 q = a / b, r = a % b;
        if (it) big |= q > 2;
        if (advance<true>(q, r, lx, ly, rx, ry, n, out)) return;
        a = b, b = r;
        q = a / b, r = a % b;
        big |= q > 2;
        if (advance<false>(q, r, rx, ry, lx, ly, n, out)) return;
        a = b, b = r;
    }
    if (big) {
        while (true) {
            u32 q = a / b, r = a % b;
            if (advance<true>(q, r, lx, ly, rx, ry, n, out)) return;
            a = b, b = r;
            q = a / b, r = a % b;
            if (advance<false>(q, r, rx, ry, lx, ly, n, out)) return;
            a = b, b = r;
        }
    } else {
        while (true) {
            if (step_small<true>(a, b, lx, ly, rx, ry, n, out)) return;
            if (step_small<false>(a, b, rx, ry, lx, ly, n, out)) return;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra7
