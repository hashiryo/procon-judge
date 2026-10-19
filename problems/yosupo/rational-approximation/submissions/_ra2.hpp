#pragma once
// 有理数近似の 2 回目。1 回目 (_ra1.hpp) の互除法を、Stern-Brocot 木の 5 回目と同じく r > b/2 の段を次の商 1 の段とまとめる
// 形にする (次の段を (b, b - r) にして、商から 1 を引く)。1 段で連が 1 つか 2 つ (商 Q と、続く 1) 進むので、向きが段ごとに
// 決まらない。そこで端を「動く端 P」と「止まった端 E」で持ち、連 t 歩は P += t E とする。続く 1 の連は E += P で、
// 向きは元に戻る。続く 1 が無ければ P と E を入れ替える。P が下の端かは low で持つ。どちらも選びで書き、分岐にしない。
// 連が N を超えるかの確かめは 1 回目と同じで、超えた連でだけ割り算で歩数を出す (続く 1 の連が超えるときは歩数は 1)。
#include "../common.hpp"

namespace ra2 {

// V = 1 なら、連を全部進めた先が N を超えるかを、P を先に進めてから P + E で確かめる (掛け算を 2 つ減らす)

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

inline void put(u64 px, u64 py, u64 ex, u64 ey, u32 low, u32* out) {
    if (low) out[0] = (u32)px, out[1] = (u32)py, out[2] = (u32)ex, out[3] = (u32)ey;
    else out[0] = (u32)ex, out[1] = (u32)ey, out[2] = (u32)px, out[3] = (u32)py;
}

template <int V = 0>
inline void solve(u32 n, u32 x, u32 y, u32* out) {
    u64 px = 0, py = 1, ex = 1, ey = 0;  // 初めは R の連なので P = L = 0/1、E = R = 1/0
    u32 low = 1, a = x, b = y, adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        if (r == 0) {  // 最後の連 (Q - 1 歩) の先が x/y
            const u64 X = px + Q * ex, Y = py + Q * ey;
            if (X <= n && Y <= n) {
                out[0] = out[2] = (u32)X, out[1] = out[3] = (u32)Y;
                return;
            }
            u64 c = ex ? (n - px) / ex : ~0ull;
            if (ey) c = std::min(c, (n - py) / ey);
            put(px + c * ex, py + c * ey, ex, ey, low, out);
            return;
        }
        if constexpr (V == 0) {
            if (px + (Q + 1ull) * ex > n || py + (Q + 1ull) * ey > n) {
                u64 c = ex ? (n - px) / ex : ~0ull;
                if (ey) c = std::min(c, (n - py) / ey);
                put(px + c * ex, py + c * ey, ex, ey, low, out);
                return;
            }
            px += Q * ex, py += Q * ey;
        } else {
            const u64 nx = px + Q * ex, ny = py + Q * ey;
            if (nx + ex > n || ny + ey > n) {
                u64 c = ex ? (n - px) / ex : ~0ull;
                if (ey) c = std::min(c, (n - py) / ey);
                put(px + c * ex, py + c * ey, ex, ey, low, out);
                return;
            }
            px = nx, py = ny;
        }
        u32 f;
        const u32 nb = pick(r, b - r, b >> 1, f);
        if (f && (ex + 2 * px > n || ey + 2 * py > n)) {  // 続く 1 の連 (E が動く) の 1 歩目の先が N を超える
            put(px, py, ex + px, ey + py, low, out);
            return;
        }
        // f なら (P, E + P)、そうでなければ (E, P) に入れ替える
        u64 m = 0ull - f;
        asm("" : "+r"(m));
        const u64 nex = ((ex + px) & m) | (px & ~m), ney = ((ey + py) & m) | (py & ~m);
        px = (px & m) | (ex & ~m), py = (py & m) | (ey & ~m);
        ex = nex, ey = ney;
        low ^= f ^ 1;
        a = b, b = nb, adj = f;
    }
}

template <int V = 0>
inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve<V>(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra2
