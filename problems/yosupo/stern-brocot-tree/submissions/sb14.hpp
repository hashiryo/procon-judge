#pragma once
// 14 回目: 13 回目 (sb13.hpp) の RANGE で、1 周の逆元の段を 3 段にする
#include "_sb5.hpp"
#include "_sb7.hpp"
#include "_sb10.hpp"
namespace sb14 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb7::decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb10::range<3, false>(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb14
using sb14::run;
