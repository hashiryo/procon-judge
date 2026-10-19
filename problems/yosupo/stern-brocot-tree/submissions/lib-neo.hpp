#pragma once
// NeoLibrary の stern_brocot.hpp を呼ぶ。中身は 20 回目 (sb20.hpp) と同じ手順 (ENCODE は r > b/2 の段をまとめる互除法、
// DECODE は vpmuludq、LCA と ANCESTOR は普通の互除法、RANGE は Kaliski の形の逆元)。
#include "../common.hpp"
#include "neo/number_theory/stern_brocot.hpp"

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: {
                u32* head = out++;
                out = sb_encode(q.x[0], q.x[1], out);
                *head = (u32)(out - head - 1);
                break;
            }
            case DECODE: {
                const u32* p = in.path.data() + q.off;
                auto [a, b] = sb_decode(p, p + q.len);
                *out++ = a, *out++ = b;
                break;
            }
            case LCA: {
                auto [f, g] = sb_lca(q.x[0], q.x[1], q.x[2], q.x[3]);
                *out++ = f, *out++ = g;
                break;
            }
            case ANCESTOR: {
                auto [f, g] = sb_ancestor(q.x[0], q.x[1], q.x[2]);
                *out++ = f, *out++ = g;
                break;
            }
            default: {
                const auto r = sb_range(q.x[0], q.x[1]);
                for (int i = 0; i < 4; ++i) *out++ = r[i];
                break;
            }
        }
    }
    return out;
}
