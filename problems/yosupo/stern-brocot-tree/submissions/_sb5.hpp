#pragma once
// Stern-Brocot 木の 5 回目。2 回目 (_sb2.hpp) の段の取り方に 4 回目の手 (ANCESTOR を祖先が無いと分かった段で止める、
// RANGE を逆元で求める) を入れ、r > b/2 で選ぶところを分岐なしにする。gcc 15 は b = f ? b - r : r を分岐に戻し、
// ループを 2 通りに複製していた (f は 3 割ほどの確率で立つので外れる)。x86 では cmova と seta を asm で書き、鎖は比べと
// 選びの 2 サイクルにする。近似分数の選びはマスクを空の asm に通して算術で書く。
#include "_sb2.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace sb5 {

// r > h なら s を、そうでなければ r を返し、f = (r > h) とする
[[gnu::always_inline]] inline u32 pick(u32 r, u32 s, u32 h, u32& f) {
#if defined(__x86_64__)
    u32 ff;
    asm("xorl %k1, %k1\n\t"
        "cmpl %k3, %k0\n\t"
        "seta %b1\n\t"
        "cmova %k2, %k0"
        : "+r"(r), "=&r"(ff)
        : "r"(s), "r"(h)
        : "cc");
    f = ff;
    return r;
#else
    f = r > h;
    return f ? s : r;
#endif
}

inline u32* encode(u32 a, u32 b, u32* out) {
    u32* head = out++;
    u32 adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) {
            *out++ = Q - 1;
            break;
        }
        u32 f;
        const u32 nb = pick(r, b - r, b >> 1, f);
        out[0] = Q, out[1] = 1;
        out += 1 + f;
        a = b, b = nb, adj = f;
    }
    *head = (u32)(out - head - 1);
    return out;
}

// 近似分数 (h1, h0) = (h_{i-1}, h_{i-2}) に商 Q の連を足し、m = -1 なら続けて商 1 の連も足す
#define SB5_STEP(h1, h0, Q, m)                         \
    do {                                               \
        const u32 hq = (Q) * h1 + h0;                  \
        h0 = (hq & (m)) | (h1 & ~(m)), h1 = hq + (h1 & (m)); \
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
        u32 f;
        const u32 nb = pick(r, b - r, b >> 1, f);
        k -= Q;
        if (f & (k == 1)) {  // 続く商 1 の連の途中 (= 連の終わり) で止まる
            const u32 h = Q * h1 + h0, kk = Q * k1 + k0;
            out[0] = h + h + h1, out[1] = kk + kk + k1;
            return;
        }
        k -= f;
        u32 m = 0u - f;
        asm("" : "+r"(m));
        SB5_STEP(h1, h0, Q, m);
        SB5_STEP(k1, k0, Q, m);
        a = b, b = nb, adj = f;
        if (k >= a) {  // (a - b adj, b) から先の深さは a - b adj - 1 以下
            out[0] = out[1] = 0;
            return;
        }
    }
}
#undef SB5_STEP

// 3 回目と同じ
inline void range(u32 a, u32 b, u32* out) {
    const u32 x = inv_mod(a, b), g = x ? x : b;
    const u32 f = (u32)(((u64)a * g - 1) / b);
    out[0] = f, out[1] = g, out[2] = a - f, out[3] = b - g;
}

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

}  // namespace sb5
