#pragma once
// Stern-Brocot 木の 18 回目。RANGE を 2 回目の r > b/2 の段をまとめる互除法に戻し、5 回目と同じく選びを分岐なしにする。
// 逆元 (9 回目) は一様な組では速いが、連分数の短い組 (端の値) と小さい値の組では互除法より遅いので、合計で比べる。
#include "_sb5.hpp"

namespace sb18 {

#define SB18_STEP(h1, h0, Q, m)                        \
    do {                                               \
        const u32 hq = (Q) * h1 + h0;                  \
        h0 = (hq & (m)) | (h1 & ~(m)), h1 = hq + (h1 & (m)); \
    } while (0)

inline void range(u32 a, u32 b, u32* out) {
    const u32 A = a, B = b;
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1, odd = 0, adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) break;
        u32 f;
        const u32 nb = sb5::pick(r, b - r, b >> 1, f);
        u32 m = 0u - f;
        asm("" : "+r"(m));
        SB18_STEP(h1, h0, Q, m);
        SB18_STEP(k1, k0, Q, m);
        odd ^= f ^ 1;
        a = b, b = nb, adj = f;
    }
    if (odd) out[0] = h1, out[1] = k1, out[2] = A - h1, out[3] = B - k1;
    else out[0] = A - h1, out[1] = B - k1, out[2] = h1, out[3] = k1;
}
#undef SB18_STEP

}  // namespace sb18
