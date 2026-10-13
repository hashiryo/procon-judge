#pragma once
// 10 回目: 9 回目 (sb9.hpp) の RANGE を、互除法と逆元を並べて回す形 (_sb10.hpp) にする。1 周に互除法 1 段と逆元 2 段
#include "_sb5.hpp"
#include "_sb7.hpp"
#include "_sb10.hpp"
namespace sb10 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb7::decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb10::range<2>(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb10
using sb10::run;
