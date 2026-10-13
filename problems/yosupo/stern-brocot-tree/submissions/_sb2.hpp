#pragma once
// Stern-Brocot 木の 2 回目。1 回目 (_sb1.hpp) の互除法を、最も近い整数への丸めの連分数 (nearest integer continued fraction)
// の段の取り方に替える。互除法の 1 段 a = q b + r で r > b/2 なら、次の段の商は必ず 1 で、余りは b - r になる。そこで
// 次の段を (b, r) でなく (b, b - r) として 2 段を 1 回の割り算で済ませる (b = r + (b - r) なので、b を b - r で割った余りは
// r を b - r で割った余りと同じで、商は 1 だけ大きい)。割り算の回数は一様な分数で 7 割ほどになる。1 段の鎖は割り算に
// r > b/2 の比べと選びが足されるが、b/2 は前もって作れるので 2 サイクルほどで済む。
// 道の連は 1 段に 1 個か 2 個 (商 Q と、続く 1) 出る。adj は前の段で (b, b - r) にしたかどうかで、そのとき商から 1 を引く。
// DECODE と LCA は 1 回目と同じ (LCA は 2 本の道の連を同じ添字で比べるので、段の取り方をそろえられない)。
#include "_sb1.hpp"

namespace sb2 {

inline u32* encode(u32 a, u32 b, u32* out) {
    u32* head = out++;
    u32 adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) {
            *out++ = Q - 1;
            break;
        }
        const u32 f = r > (b >> 1);
        out[0] = Q, out[1] = 1;
        out += 1 + f;
        const u32 s = b - r;
        a = b, b = f ? s : r, adj = f;
    }
    *head = (u32)(out - head - 1);
    return out;
}

// 近似分数 (h1, h0) = (h_{i-1}, h_{i-2}) に商 Q の連を足し、f なら続けて商 1 の連も足す
#define SB2_STEP(h1, h0, Q, f)                     \
    do {                                           \
        const u32 hq = (Q) * h1 + h0;              \
        const u32 m = 0u - (f);                    \
        h0 = (hq & m) | (h1 & ~m), h1 = hq + (h1 & m); \
    } while (0)

inline void ancestor(u32 k, u32 a, u32 b, u32* out) {
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
        SB2_STEP(h1, h0, Q, f);
        SB2_STEP(k1, k0, Q, f);
        const u32 s = b - r;
        a = b, b = f ? s : r, adj = f;
    }
}

inline void range(u32 a, u32 b, u32* out) {
    const u32 A = a, B = b;
    u32 h1 = 1, k1 = 0, h0 = 0, k0 = 1, odd = 0, adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) break;
        const u32 f = r > (b >> 1);
        SB2_STEP(h1, h0, Q, f);
        SB2_STEP(k1, k0, Q, f);
        odd ^= f ^ 1;
        const u32 s = b - r;
        a = b, b = f ? s : r, adj = f;
    }
    if (odd) out[0] = h1, out[1] = k1, out[2] = A - h1, out[3] = B - k1;
    else out[0] = A - h1, out[1] = B - k1, out[2] = h1, out[3] = k1;
}
#undef SB2_STEP

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = encode(q.x[0], q.x[1], out); break;
            case DECODE: sb1::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}

}  // namespace sb2
