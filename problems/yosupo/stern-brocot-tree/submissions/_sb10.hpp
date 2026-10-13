#pragma once
// Stern-Brocot 木の 10 回目。RANGE を、互除法 (1 回目) と逆元 (9 回目の Kaliski の形) を 1 つのループで並べて回し、先に
// 終わったほうで答える。互除法は割り算器、逆元は ALU を使うので並んで進む。互除法の 1 段は割り算 1 回ぶん (12 サイクル
// ほど)、逆元の 1 段は引き算、tzcnt、シフトの 5 サイクルほどなので、1 周に互除法 1 段と逆元 S 段を置く。一様な組は
// 逆元が先に (19 段ほど)、連分数の短い組 (端の値) は互除法が先に終わる。G が false なら、逆元の側の桁数の差が大きいときの
// 割り算を省く (その組は互除法の側が早く終わるので、割り算器と分岐を互除法に譲る)。
#include "_sb9.hpp"

namespace sb10 {

template <int S, bool G = true>
inline void range(u32 a0, u32 b0, u32* out) {
    using namespace sb9;
    // 逆元の側 (9 回目と同じ前処理)
    const u32 sw = ~b0 & 1;
    u32 msk = 0u - sw;
    asm("" : "+r"(msk));
    const u32 B = (a0 & msk) | (b0 & ~msk), A0 = a0 ^ b0 ^ B;
    const u64 Binv = inv64(B);
    // 互除法の側
    u32 x = a0, y = b0, h1 = 1, k1 = 0, h0 = 0, k0 = 1, odd = 0;
    // 逆元の側の初めの値 (桁数の差が大きいときの割り算 1 回を含む)
    u32 A = A0;
    if (G && A > B && __builtin_clz(B) - __builtin_clz(A) > GAP) A %= B;
    u32 kx;  // 逆元 [1, B]
    if (A == 0) {
        kx = B;
    } else {
        int k = __builtin_ctz(A);
        u32 a = B, b = A >> k, ca = 0, cb = 1, neg = 0;
        if (G && __builtin_clz(b) - __builtin_clz(a) > GAP) {
            const u32 q = a / b, r = a - q * b;
            if (r == 0) a = b, ca = q - 1;
            else {
                const int c = __builtin_ctz(r);
                a = r >> c, ca = q, cb <<= c, k += c;
            }
        }
        while (true) {
            // 互除法の 1 段
            const u32 q = x / y, r = x % y;
            if (r == 0) {
                if (odd) out[0] = h1, out[1] = k1, out[2] = a0 - h1, out[3] = b0 - k1;
                else out[0] = a0 - h1, out[1] = b0 - k1, out[2] = h1, out[3] = k1;
                return;
            }
            const u32 h = q * h1 + h0, kk = q * k1 + k0;
            h0 = h1, k0 = k1, h1 = h, k1 = kk;
            x = y, y = r, odd ^= 1;
            // 逆元の S 段
            bool done = false;
            for (int s = 0; s < S; ++s) {
                if (a == b) {
                    done = true;
                    break;
                }
                const u32 d = a - b, e = b - a;
                const int c = __builtin_ctz(d);
                const u32 mn = a < b ? a : b, t = a > b ? d : e, cm = a > b ? cb : ca;
                neg += a < b;
                ca += cb, cb = cm << c, k += c;
                a = t >> c, b = mn;
            }
            if (done) break;
        }
        u64 v = redc(cb, k, B, Binv);
        v = v >= B ? v - B : v;
        kx = (u32)((neg & 1) ? B - v : (v ? v : B));
    }
    const u32 ky = (u32)(((u64)A0 * kx - 1) * Binv);
    const u32 g = (kx & ~msk) | ((b0 - ky) & msk), f = (ky & ~msk) | ((a0 - kx) & msk);
    out[0] = f, out[1] = g, out[2] = a0 - f, out[3] = b0 - g;
}

}  // namespace sb10
