#pragma once
// 有理数近似の 6 回目。1 回目 (_ra1.hpp) の互除法に、商の小さい分数向けの道を足す。最初の 2 組 (R、L、R、L の 4 連) は
// 割り算で進め、q_1、q_2、q_3 がどれも 2 以下なら、残りを「商 2 以下は引き算と cmov で出し、3 以上だけ割る」形で進める。
// 商 1 と 2 だけの段は鎖が引き算、比べ、cmov の 3 サイクルで、割り算の 12 サイクルより短い。3 以上かの分岐は、
// 商の小さい分数ではほとんど外れない。一様な分数は 2 割ほどがこちらに入り、そこでは分岐が外れる分だけ遅れる。
// 端の更新も、商が 1 か 2 なので掛け算を lea と cmov にできるが、ここでは 1 回目と同じ掛け算のまま (鎖は 4 サイクル)。
#include "../common.hpp"

namespace ra6 {

// 連 t 歩を進める (動く端 (mx, my)、止まった端 (fx, fy))。答えが決まったら書いて true を返す。low は動く端が下の端か
template <bool low>
[[gnu::always_inline]] inline bool advance(u32 q, u32 r, u64& mx, u64& my, u64 fx, u64 fy, u32 n, u32* out) {
    const u64 t = q - (r == 0);
    if (mx + (t + 1) * fx <= n && my + (t + 1) * fy <= n) {
        mx += t * fx, my += t * fy;
        if (r != 0) return false;
        out[0] = out[2] = (u32)(mx + fx), out[1] = out[3] = (u32)(my + fy);
        return true;
    }
    u64 c = fx ? (n - mx) / fx : ~0ull;
    if (fy) c = std::min(c, (n - my) / fy);
    mx += c * fx, my += c * fy;
    if (low) out[0] = (u32)mx, out[1] = (u32)my, out[2] = (u32)fx, out[3] = (u32)fy;
    else out[0] = (u32)fx, out[1] = (u32)fy, out[2] = (u32)mx, out[3] = (u32)my;
    return true;
}

// a > b のときの商と余り。商 2 以下は引き算と cmov、3 以上は割り算
[[gnu::always_inline]] inline void qr_small(u32 a, u32 b, u32& q, u32& r) {
    if ((u64)a >= 3ull * b) [[unlikely]] {
        asm volatile("");  // clang はこの割り算を毎段計算して選ぶ形にしていた。volatile の asm で分岐のまま残させる
        q = a / b, r = a % b;
        return;
    }
    u32 d = a - b;
#if defined(__x86_64__)
    u32 ge;
    asm("xorl %k1, %k1\n\t"
        "cmpl %k2, %k0\n\t"
        "setae %b1\n\t"
        "cmovae %k3, %k0"
        : "+r"(d), "=&r"(ge)
        : "r"(b), "r"(d - b)
        : "cc");
    q = 1 + ge, r = d;
#else
    const u32 ge = d >= b;
    q = 1 + ge, r = ge ? d - b : d;
#endif
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
            u32 q, r;
            qr_small(a, b, q, r);
            if (advance<true>(q, r, lx, ly, rx, ry, n, out)) return;
            a = b, b = r;
            qr_small(a, b, q, r);
            if (advance<false>(q, r, rx, ry, lx, ly, n, out)) return;
            a = b, b = r;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra6
