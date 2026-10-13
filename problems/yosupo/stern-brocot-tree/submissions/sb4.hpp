#pragma once
// 4 回目: 3 回目 (sb3.hpp) の ANCESTOR で、深さ k の祖先が無いと分かった段で止める。互除法の組 (x, y) (x ≥ y、既約) から
// 先の連の長さの和は x 以下 (引き算の互除法で数えると、1 回の引き算で x + y が 1 以上減り、1 で終わる。帰納法で
// q + Σ(y, r) ≤ q + y ≤ q y + r = x)。最後の連は 1 だけ短いので、先の深さは x - 1 以下で、残りの深さが x 以上なら祖先は無い。
#include "_sb2.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace sb4 {

// 3 回目と同じ
inline void range(u32 a, u32 b, u32* out) {
    const u32 x = inv_mod(a, b), g = x ? x : b;
    const u32 f = (u32)(((u64)a * g - 1) / b);
    out[0] = f, out[1] = g, out[2] = a - f, out[3] = b - g;
}

#define SB4_STEP(h1, h0, Q, f)                     \
    do {                                           \
        const u32 hq = (Q) * h1 + h0;              \
        const u32 m = 0u - (f);                    \
        h0 = (hq & m) | (h1 & ~m), h1 = hq + (h1 & m); \
    } while (0)

inline void ancestor(u32 k, u32 a, u32 b, u32* out) {
    if (k >= (a > b ? a : b)) {
        out[0] = out[1] = 0;
        return;
    }
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1, adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) {
            if (k < Q) out[0] = k * h1 + h0 + h1, out[1] = k * k1 + k0 + k1;
            else out[0] = out[1] = 0;
            return;
        }
        if (k <= Q) {
            out[0] = k * h1 + h0 + h1, out[1] = k * k1 + k0 + k1;
            return;
        }
        const u32 f = r > (b >> 1);
        k -= Q;
        if (f & (k == 1)) {  // 続く商 1 の連の途中 (= 連の終わり) で止まる
            const u32 h = Q * h1 + h0, kk = Q * k1 + k0;
            out[0] = h + h + h1, out[1] = kk + kk + k1;
            return;
        }
        k -= f;
        SB4_STEP(h1, h0, Q, f);
        SB4_STEP(k1, k0, Q, f);
        const u32 s = b - r;
        a = b, b = f ? s : r, adj = f;
        if (k >= a) {  // (a - b adj, b) から先の深さは a - b adj - 1 以下
            out[0] = out[1] = 0;
            return;
        }
    }
}
#undef SB4_STEP

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb2::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb1::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}

}  // namespace sb4
using sb4::run;
