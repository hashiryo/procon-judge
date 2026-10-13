#pragma once
// Stern-Brocot 木の 1 回目。どの問も a/b の連分数展開 [q_0; q_1, ..., q_m] (互除法の商) で解く。a/b への道は
// R^{q_0} L^{q_1} R^{q_2} ... で、最後の連だけ q_m - 1 になる (q_0 は 0 でもよい)。互除法は 32 bit の割り算で回し、
// 余りが次の割る数なので 1 段の鎖は割り算 1 回ぶん (商は x86 の div で余りと一緒に出る)。
// 道の途中の節点は、連分数の近似分数 h_i / k_i (h_i = q_i h_{i-1} + h_{i-2}、h_{-1} / k_{-1} = 1/0、h_{-2} / k_{-2} = 0/1)
// から作る。連 t_0, ..., t_j の道の行き先は (h_j + h_{j-1}) / (k_j + k_{j-1}) で、h_j は q_j を t_j に替えて作る。
// 近似分数の掛け算と足し算は割り算の鎖と並んで進む。答えは 2^32 未満なので、途中は u32 の桁あふれのまま計算してよい。
#include "../common.hpp"

namespace sb1 {

inline u32* encode(u32 a, u32 b, u32* out) {
    u32* head = out++;
    while (true) {
        const u32 q = a / b, r = a % b;
        if (r == 0) {
            *out++ = q - 1;
            break;
        }
        *out++ = q;
        a = b, b = r;
    }
    *head = (u32)(out - head - 1);
    return out;
}

// 連の長さ p[0, len) (R から) の行き先
inline void decode(const u32* p, u32 len, u32* out) {
    u32 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 i = 0;
    for (; i + 1 < len; i += 2) {
        lx += p[i] * rx, ly += p[i] * ry;
        rx += p[i + 1] * lx, ry += p[i + 1] * ly;
    }
    if (i < len) lx += p[i] * rx, ly += p[i] * ry;
    out[0] = lx + rx, out[1] = ly + ry;
}

// 2 つの道を同じ段で進め、連が初めて食い違う (かどちらかの道が終わる) 段で、短いほうの連で止まる
inline void lca(u32 a, u32 b, u32 c, u32 d, u32* out) {
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1;
    while (true) {
        const u32 q = a / b, r = a % b;
        const u32 s = c / d, t = c % d;
        const u32 T = q - (r == 0), S = s - (t == 0);
        if (T != S || r == 0 || t == 0) {
            const u32 m = T < S ? T : S;
            out[0] = m * h1 + h0 + h1, out[1] = m * k1 + k0 + k1;
            return;
        }
        const u32 h = q * h1 + h0, k = q * k1 + k0;
        h0 = h1, k0 = k1, h1 = h, k1 = k;
        a = b, b = r, c = d, d = t;
    }
}

// 深さ k の祖先。道を連ごとに進み、残りの深さが連の長さ以下になった連の途中で止まる
inline void ancestor(u32 k, u32 a, u32 b, u32* out) {
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
    }
}

// 部分木の区間の端は h_{m-1} / k_{m-1} と (a - h_{m-1}) / (b - k_{m-1})。近似分数は添字が偶数なら a/b より小さい
inline void range(u32 a, u32 b, u32* out) {
    const u32 A = a, B = b;
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1, odd = 0;
    while (true) {
        const u32 q = a / b, r = a % b;
        if (r == 0) break;
        const u32 h = q * h1 + h0, k = q * k1 + k0;
        h0 = h1, k0 = k1, h1 = h, k1 = k;
        a = b, b = r, odd ^= 1;
    }
    // m = odd (の偶奇)。h1 / k1 = h_{m-1} / k_{m-1}
    if (odd) out[0] = h1, out[1] = k1, out[2] = A - h1, out[3] = B - k1;
    else out[0] = A - h1, out[1] = B - k1, out[2] = h1, out[3] = k1;
}

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = encode(q.x[0], q.x[1], out); break;
            case DECODE: decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}

}  // namespace sb1
