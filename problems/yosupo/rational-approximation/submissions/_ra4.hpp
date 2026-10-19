#pragma once
// 有理数近似の 4 回目。3 回目 (_ra2.hpp の V = 1) と同じ手順で、出口の確かめを 1 回の分岐にまとめる。gcc 15 は
// f && (続く 1 の連が N を超える) を f の分岐にしていた (f は 3 割ほどの確率で立つので外れる)。2 つの成分の確かめは
// max を取ってから比べ、f との AND を取ってから分岐する。f は空の asm に通して、gcc に分岐へ戻させない。
#include "_ra2.hpp"

namespace ra4 {

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    using ra2::put;
    u64 px = 0, py = 1, ex = 1, ey = 0;
    u32 low = 1, a = x, b = y, adj = 0;
    while (true) {
        const u32 q = a / b, r = a % b, Q = q - adj;
        const u64 nx = px + Q * ex, ny = py + Q * ey;
        if (r == 0) {  // 最後の連 (Q - 1 歩) の先が x/y
            if (std::max(nx, ny) <= n) {
                out[0] = out[2] = (u32)nx, out[1] = out[3] = (u32)ny;
                return;
            }
            u64 c = ex ? (n - px) / ex : ~0ull;
            if (ey) c = std::min(c, (n - py) / ey);
            put(px + c * ex, py + c * ey, ex, ey, low, out);
            return;
        }
        if (std::max(nx + ex, ny + ey) > n) {
            u64 c = ex ? (n - px) / ex : ~0ull;
            if (ey) c = std::min(c, (n - py) / ey);
            put(px + c * ex, py + c * ey, ex, ey, low, out);
            return;
        }
        px = nx, py = ny;
        u32 f;
        const u32 nb = ra2::pick(r, b - r, b >> 1, f);
        asm("" : "+r"(f));
        const u32 over = std::max(ex + 2 * px, ey + 2 * py) > n;
        if (f & over) {  // 続く 1 の連 (E が動く) の 1 歩目の先が N を超える
            put(px, py, ex + px, ey + py, low, out);
            return;
        }
        u64 m = 0ull - f;
        const u64 nex = ((ex + px) & m) | (px & ~m), ney = ((ey + py) & m) | (py & ~m);
        px = (px & m) | (ex & ~m), py = (py & m) | (ey & ~m);
        ex = nex, ey = ney;
        low ^= f ^ 1;
        a = b, b = nb, adj = f;
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra4
