#pragma once
// 15 回目: 9 回目 (sb9.hpp) の ENCODE を、前と後ろの 2 本の互除法にする形 (_sb15.hpp) にする。逆元が求まるまでは 1 周に前の互除法 1 段と逆元 2 段
#include "_sb5.hpp"
#include "_sb7.hpp"
#include "_sb15.hpp"
namespace sb15 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb15::encode<2>(q.x[0], q.x[1], out); break;
            case DECODE: sb7::decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb9::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb15
using sb15::run;
