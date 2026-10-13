#pragma once
// Stern-Brocot 木の 6 回目。1 回目 (_sb1.hpp) の普通の互除法のまま、4 回目の 2 つの手 (ANCESTOR を祖先が無いと分かった段で止める、
// RANGE を逆元で求める) だけを入れる。5 回目 (r > b/2 の段をまとめる) と比べて、まとめる手の効き目を分けて見る。
#include "_sb1.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace sb6 {

inline void ancestor(u32 k, u32 a, u32 b, u32* out) {
    if (k >= (a > b ? a : b)) {
        out[0] = out[1] = 0;
        return;
    }
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1;
    while (true) {
        const u32 q = a / b, r = a % b;
        const u32 T = q - (r == 0);
        if (k <= T) {
            out[0] = k * h1 + h0 + h1, out[1] = k * k1 + k0 + k1;
            return;
        }
        if (r == 0) {
            out[0] = out[1] = 0;
            return;
        }
        k -= q;
        const u32 h = q * h1 + h0, kk = q * k1 + k0;
        h0 = h1, k0 = k1, h1 = h, k1 = kk;
        a = b, b = r;
        if (k >= a) {  // (a, b) から先の深さは a - 1 以下
            out[0] = out[1] = 0;
            return;
        }
    }
}

// 3 回目と同じ
inline void range(u32 a, u32 b, u32* out) {
    const u32 x = inv_mod(a, b), g = x ? x : b;
    const u32 f = (u32)(((u64)a * g - 1) / b);
    out[0] = f, out[1] = g, out[2] = a - f, out[3] = b - g;
}

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb1::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb1::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}

}  // namespace sb6
