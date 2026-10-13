#pragma once
// Stern-Brocot 木の 7 回目。DECODE を書き直す。1 回目の decode は gcc も clang も (lx, ly) と (rx, ry) を xmm に詰めて
// vpmulld で掛けていた (SLP のベクトル化)。vpmulld は Intel で 10 サイクル待つので、1 連の鎖が imul と add の 4 サイクルから
// 11 サイクルに延びる。値を空の asm に通してスカラーの imul のままにする。V は
//   0: 1 回目と同じ形 (1 連ごとに掛けて足す)
//   1: 2 連をまとめる。R の連 n と L の連 m を続けると、L' = L + n R、R' = (n m + 1) R + m L で、どちらも (L, R) だけから
//      作れるので、鎖は 2 連で掛け算と足し算 1 回ずつになる (掛け算は 4 回から 7 回に増える)
#include "_sb1.hpp"

namespace sb7 {

template <int V>
inline void decode(const u32* p, u32 len, u32* out) {
    u32 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 i = 0;
    for (; i + 1 < len; i += 2) {
        const u32 n = p[i], m = p[i + 1];
        if constexpr (V == 0) {
            lx += n * rx, ly += n * ry;
            rx += m * lx, ry += m * ly;
        } else {
            const u32 c = n * m + 1;
            const u32 nlx = lx + n * rx, nly = ly + n * ry;
            rx = c * rx + m * lx, ry = c * ry + m * ly;
            lx = nlx, ly = nly;
        }
        asm("" : "+r"(lx), "+r"(ly), "+r"(rx), "+r"(ry));
    }
    if (i < len) lx += p[i] * rx, ly += p[i] * ry;
    out[0] = lx + rx, out[1] = ly + ry;
}

}  // namespace sb7
