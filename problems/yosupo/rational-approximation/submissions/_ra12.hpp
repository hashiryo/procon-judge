#pragma once
// 有理数近似の 12 回目。11 回目 (_ra11.hpp) の道を 3 段にする。整数部分の次からは商 2 以下の道 (引き算と cmov 1 段) で
// 進み、商 3 が出たら商 3 まで選ぶ道 (a ≥ 2b と a ≥ 3b の比べと cmov 2 段) へ、商 4 以上が出たら割り算の道へ移る。
// 商 3 まで選ぶ道でも商 4 以上が出たら割り算の道へ移る。small_run_length_02 (商が 1、2、3 で均等) を割り算なしで
// 進めるため。10 回目は gcc が a - 2b と a - 3b を (a - b) - b と順に引いて鎖が延びたので、2b と 3b を空の asm に
// 通して書き換えさせない。端の更新は商 - 1 の 2 つの比べのマスクで足す。3b < 2^32 が前提。
#include "_ra11.hpp"

namespace ra12 {

using ra11::advance;
enum { CONT, DONE, BIG };

// 商 2 以下の道の 1 段 (a > b)。商 3 以上なら何も進めずに BIG を返す
template <bool low>
[[gnu::always_inline]] inline int step2(u32& a, u32& b, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out) {
    u32 d = a - b, ge;
#if defined(__x86_64__)
    asm("xorl %k1, %k1\n\tcmpl %k2, %k0\n\tsetae %b1\n\tcmovae %k3, %k0" : "+r"(d), "=&r"(ge) : "r"(b), "r"(d - b) : "cc");
#else
    ge = d >= b;
    d = ge ? d - b : d;
#endif
    if (d >= b) [[unlikely]] return BIG;
    if (d == 0) [[unlikely]] return advance<low>(1 + ge, d, mx, my, fx, fy, n, sw, out), DONE;
    const u64 m = 0ull - ge;
    const u64 nx = mx + fx + (fx & m);
    if (nx + fx > n) [[unlikely]] return advance<low>(1 + ge, d, mx, my, fx, fy, n, sw, out), DONE;
    mx = nx, my += fy + (fy & m);
    a = b, b = d;
    return CONT;
}

// 商 3 以下の道の 1 段 (a > b)。商 4 以上なら何も進めずに BIG を返す
template <bool low>
[[gnu::always_inline]] inline int step3(u32& a, u32& b, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out) {
    u32 b2 = b + b, b3 = b2 + b;
    asm("" : "+r"(b2), "+r"(b3));
    u32 d = a - b, g1, g2;
#if defined(__x86_64__)
    asm("xorl %k1, %k1\n\t"
        "xorl %k2, %k2\n\t"
        "cmpl %k4, %k3\n\t"
        "setae %b1\n\t"
        "cmovae %k5, %k0\n\t"
        "cmpl %k6, %k3\n\t"
        "setae %b2\n\t"
        "cmovae %k7, %k0"
        : "+r"(d), "=&r"(g1), "=&r"(g2)
        : "r"(a), "r"(b2), "r"(a - b2), "r"(b3), "r"(a - b3)
        : "cc");
#else
    g1 = a >= b2, g2 = a >= b3;
    d = g2 ? a - b3 : g1 ? a - b2 : d;
#endif
    if (d >= b) [[unlikely]] return BIG;
    if (d == 0) [[unlikely]] return advance<low>(1 + g1 + g2, d, mx, my, fx, fy, n, sw, out), DONE;
    const u64 m1 = 0ull - g1, m2 = 0ull - g2;
    const u64 nx = mx + fx + ((fx & m1) + (fx & m2));
    if (nx + fx > n) [[unlikely]] return advance<low>(1 + g1 + g2, d, mx, my, fx, fy, n, sw, out), DONE;
    mx = nx, my += fy + ((fy & m1) + (fy & m2));
    a = b, b = d;
    return CONT;
}

// 割り算の 1 段。答えが決まったら true。商を q に返す
template <bool low>
[[gnu::always_inline]] inline bool stepd(u32& a, u32& b, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out, u32& q) {
    q = a / b;
    const u32 r = a % b;
    if (advance<low>(q, r, mx, my, fx, fy, n, sw, out)) return true;
    a = b, b = r;
    return false;
}

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    const u32 sw = x < y;
    u32 a = sw ? y : x, b = sw ? x : y, q;
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    if (stepd<true>(a, b, lx, ly, rx, ry, n, sw, out, q)) return;
    // 商 2 以下の道 (L の連から)
    while (true) {
        if (int s = step2<false>(a, b, rx, ry, lx, ly, n, sw, out); s != CONT) {
            if (s == DONE) return;
            asm volatile("");
            if (stepd<false>(a, b, rx, ry, lx, ly, n, sw, out, q)) return;
            if (q == 3) goto s3_R;
            goto d_R;
        }
        if (int s = step2<true>(a, b, lx, ly, rx, ry, n, sw, out); s != CONT) {
            if (s == DONE) return;
            asm volatile("");
            if (stepd<true>(a, b, lx, ly, rx, ry, n, sw, out, q)) return;
            if (q == 3) goto s3_L;
            goto d_L;
        }
    }
    // 商 3 以下の道
s3_R:
    while (true) {
        if (int s = step3<true>(a, b, lx, ly, rx, ry, n, sw, out); s != CONT) {
            if (s == DONE) return;
            asm volatile("");
            if (stepd<true>(a, b, lx, ly, rx, ry, n, sw, out, q)) return;
            goto d_L;
        }
    s3_L:
        if (int s = step3<false>(a, b, rx, ry, lx, ly, n, sw, out); s != CONT) {
            if (s == DONE) return;
            asm volatile("");
            if (stepd<false>(a, b, rx, ry, lx, ly, n, sw, out, q)) return;
            goto d_R;
        }
    }
    // 割り算の道
d_R:
    while (true) {
        if (stepd<true>(a, b, lx, ly, rx, ry, n, sw, out, q)) return;
    d_L:
        if (stepd<false>(a, b, rx, ry, lx, ly, n, sw, out, q)) return;
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra12
