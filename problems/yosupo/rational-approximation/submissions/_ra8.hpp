#pragma once
// 有理数近似の 8 回目。7 回目 (_ra7.hpp) の 1 項あたりの命令を減らす。
// - x < y なら x と y を入れ替えて解き、答えの分子と分母、下と上を入れ替えて返す (x/y ↔ y/x で問題は同じ形)。x ≥ y なら
//   道の節点と端はどれも 1 以上 (分子 ≥ 分母) なので、N を超えるかは分子だけ見ればよい。
// - 商の小さい道の商 3 以上の判定は、引き算と cmov の後の r ≥ b にする (d = a - b < b なら r = d < b、そうでなければ
//   r = d - b で、r ≥ b と商 3 以上が同じ)。前もって 3b を作って比べる分がなくなる。
#include "../common.hpp"

namespace ra8 {

// 連 t 歩を進める (動く端 (mx, my)、止まった端 (fx, fy))。low は動く端が下の端か。答えは (下の分子, 下の分母, 上の分子,
// 上の分母) を、sw なら (上の分母, 上の分子, 下の分母, 下の分子) にして書く
template <bool low>
[[gnu::always_inline]] inline bool advance(u32 q, u32 r, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out) {
    const u64 t = q - (r == 0);
    if (mx + (t + 1) * fx <= n) {
        mx += t * fx, my += t * fy;
        if (r != 0) return false;
        const u32 X = (u32)(mx + fx), Y = (u32)(my + fy);
        if (sw) out[0] = out[2] = Y, out[1] = out[3] = X;
        else out[0] = out[2] = X, out[1] = out[3] = Y;
        return true;
    }
    const u64 c = (n - mx) / fx;  // fx ≥ 1 (x ≥ y なら止まった端の分子は 1 以上)
    mx += c * fx, my += c * fy;
    u32 lo[2] = {(u32)mx, (u32)my}, hi[2] = {(u32)fx, (u32)fy};
    if (!low) std::swap(lo, hi);
    if (sw) out[0] = hi[1], out[1] = hi[0], out[2] = lo[1], out[3] = lo[0];
    else out[0] = lo[0], out[1] = lo[1], out[2] = hi[0], out[3] = hi[1];
    return true;
}

// 商の小さい道の 1 段 (a > b)。答えが決まったら true
template <bool low>
[[gnu::always_inline]] inline bool step_small(u32& a, u32& b, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32 sw, u32* out) {
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
    if (d >= b) [[unlikely]] {  // 商 3 以上
        asm volatile("");
        const u32 q = a / b, r = a % b;
        if (advance<low>(q, r, mx, my, fx, fy, n, sw, out)) return true;
        a = b, b = r;
        return false;
    }
    const u32 r = d;
    if (r == 0) [[unlikely]] return advance<low>(1 + ge, r, mx, my, fx, fy, n, sw, out);
    const u64 m = 0ull - ge;
    const u64 nx = mx + fx + (fx & m);
    if (nx + fx > n) [[unlikely]] return advance<low>(1 + ge, r, mx, my, fx, fy, n, sw, out);
    mx = nx, my += fy + (fy & m);
    a = b, b = r;
    return false;
}

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    const u32 sw = x < y;
    u32 a = sw ? y : x, b = sw ? x : y;
    // a ≥ b なので最初は R の連 (q ≥ 1)
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 big = 0;
    for (int it = 0; it < 2; ++it) {
        u32 q = a / b, r = a % b;
        if (it) big |= q > 2;
        if (advance<true>(q, r, lx, ly, rx, ry, n, sw, out)) return;
        a = b, b = r;
        q = a / b, r = a % b;
        big |= q > 2;
        if (advance<false>(q, r, rx, ry, lx, ly, n, sw, out)) return;
        a = b, b = r;
    }
    if (big) {
        while (true) {
            u32 q = a / b, r = a % b;
            if (advance<true>(q, r, lx, ly, rx, ry, n, sw, out)) return;
            a = b, b = r;
            q = a / b, r = a % b;
            if (advance<false>(q, r, rx, ry, lx, ly, n, sw, out)) return;
            a = b, b = r;
        }
    } else {
        while (true) {
            if (step_small<true>(a, b, lx, ly, rx, ry, n, sw, out)) return;
            if (step_small<false>(a, b, rx, ry, lx, ly, n, sw, out)) return;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra8
