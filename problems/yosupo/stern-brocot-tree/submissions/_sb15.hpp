#pragma once
// Stern-Brocot 木の 15 回目。ENCODE の互除法を前と後ろの 2 本の鎖にする。a/b = [q_0; q_1, ..., q_m] の近似分数の分母を
// k_i とすると、b / k_{m-1} = [q_m; q_{m-1}, ..., q_1] なので、(b, k_{m-1}) の互除法は連分数の項を後ろから出す。k_{m-1} は
// a/b の区間の端の分母の小さいほう (q_m ≥ 2 なので k_{m-1} ≤ b/2) で、9 回目の RANGE と同じ逆元から求まる。
// 1 周目からは前の互除法 1 段と逆元 S 段を並べ、逆元が求まったら前と後ろの互除法を 1 段ずつ並べる。前の鎖は近似分数の
// 分母の組 (k_j, k_{j-1}) を持ち、後ろの鎖の組 (k_{t-1}, k_{t-2}) と一致したら (j = t - 1)、項は前の q_0..q_j と後ろの
// q_{j+1}..q_m で全部そろう。後ろの鎖は 1 段ずつ確かめるので、組は必ず一度ぴったり一致する。
// ただし q_1 = 1 なら k_1 = k_0 = 1 で、(k_2, k_1) の互除法は q_2 と q_1 をまとめた q_2 + 1 を出してしまう。後ろの鎖に q_2 より
// 前を作らせないため、前の鎖が q_0, q_1, q_2 を出すまでは後ろの鎖を動かさない (後ろの鎖が出すのは q_{j+1} で、j ≥ 2)。
#include "_sb9.hpp"

namespace sb15 {

template <int S>
inline u32* encode(u32 a, u32 b, u32* out) {
    using namespace sb9;
    u32* head = out++;
    // 逆元の側 (9 回目の RANGE と同じ前処理)。b が奇数なら x = a^{-1} mod b = g、偶数なら x = b^{-1} mod a = h
    const u32 sw = ~b & 1;
    u32 msk = 0u - sw;
    asm("" : "+r"(msk));
    const u32 B = (a & msk) | (b & ~msk), A0 = a ^ b ^ B;
    const u64 Binv = inv64(B);
    u32 A = A0;
    if (A > B && __builtin_clz(B) - __builtin_clz(A) > GAP) A %= B;
    // 前の鎖
    u32 x = a, y = b, k1 = 0, k0 = 1;  // (k1, k0) = (k_{j}, k_{j-1})、初めは (k_{-1}, k_{-2})
    u32 back[MAX_RUNS + 1];
    u32 nb = 0;
    if (A != 0) {
        int k = __builtin_ctz(A);
        u32 ka = B, kb = A >> k, ca = 0, cb = 1, neg = 0;
        if (__builtin_clz(kb) - __builtin_clz(ka) > GAP) {
            const u32 q = ka / kb, r = ka - q * kb;
            if (r == 0) ka = kb, ca = q - 1;
            else {
                const int c = __builtin_ctz(r);
                ka = r >> c, ca = q, cb <<= c, k += c;
            }
        }
        while (true) {
            const u32 q = x / y, r = x % y;
            if (r == 0) {
                *out++ = q - 1;
                *head = (u32)(out - head - 1);
                return out;
            }
            *out++ = q;
            const u32 kk = q * k1 + k0;
            k0 = k1, k1 = kk;
            x = y, y = r;
            bool done = false;
            for (int s = 0; s < S; ++s) {
                if (ka == kb) {
                    done = true;
                    break;
                }
                const u32 d = ka - kb, e = kb - ka;
                const int c = __builtin_ctz(d);
                const u32 mn = ka < kb ? ka : kb, t = ka > kb ? d : e, cm = ka > kb ? cb : ca;
                neg += ka < kb;
                ca += cb, cb = cm << c, k += c;
                ka = t >> c, kb = mn;
            }
            if (done) break;
        }
        u64 v = redc(cb, k, B, Binv);
        v = v >= B ? v - B : v;
        const u32 ix = (u32)((neg & 1) ? B - v : (v ? v : B));
        const u32 iy = (u32)(((u64)A0 * ix - 1) * Binv);
        const u32 g = (ix & ~msk) | ((b - iy) & msk);  // 区間の下端の分母
        // 前の鎖に q_2 まで出させる (head の次から 3 項)
        while (out - head < 4) {
            const u32 q = x / y, r = x % y;
            if (r == 0) {
                *out++ = q - 1;
                *head = (u32)(out - head - 1);
                return out;
            }
            *out++ = q;
            const u32 kk = q * k1 + k0;
            k0 = k1, k1 = kk;
            x = y, y = r;
        }
        // 後ろの鎖 (X, Y) = (k_m, k_{m-1}) = (b, min(g, b - g))
        u32 X = b, Y = g < b - g ? g : b - g;
        while (true) {
            if (k1 == X && k0 == Y) break;
            {
                const u32 q = X / Y, r = X % Y;
                back[nb++] = q;
                X = Y, Y = r;
            }
            if (k1 == X && k0 == Y) break;
            {
                const u32 q = x / y, r = x % y;
                *out++ = q;
                const u32 kk = q * k1 + k0;
                k0 = k1, k1 = kk;
                x = y, y = r;
            }
        }
    } else {
        // b = 1 か a = 1 (法が 1)。前の鎖だけで終わる
        while (true) {
            const u32 q = x / y, r = x % y;
            if (r == 0) {
                *out++ = q - 1;
                *head = (u32)(out - head - 1);
                return out;
            }
            *out++ = q;
            x = y, y = r;
        }
    }
    // 後ろの項を逆に並べて足す。最後の項 (q_m) から 1 を引く
    back[0] -= 1;
    for (u32 i = nb; i-- > 0;) *out++ = back[i];
    *head = (u32)(out - head - 1);
    return out;
}

}  // namespace sb15
