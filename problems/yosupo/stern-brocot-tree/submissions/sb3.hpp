#pragma once
// 3 回目: 2 回目 (_sb2.hpp) の RANGE を、互除法でなく逆元で求める。区間の下端 f/g は a g - b f = 1 を満たすので、
// g は a の mod b の逆元 (b = 1 のときは g = 1)、f = (a g - 1) / b。逆元は NeoLibrary の inv_mod (Stein の方法の拡張を
// Kaliski の形で書いたもの、割り算なし) に任せ、f は 64 bit の割り算 1 回で出す。
#include "_sb2.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace sb3 {

inline void range(u32 a, u32 b, u32* out) {
    const u32 x = inv_mod(a, b), g = x ? x : b;
    const u32 f = (u32)(((u64)a * g - 1) / b);
    out[0] = f, out[1] = g, out[2] = a - f, out[3] = b - g;
}

inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb2::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb1::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb2::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}

}  // namespace sb3
using sb3::run;
