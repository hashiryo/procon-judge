#pragma once
// 7 回目: 6 回目 (sb6.hpp) の DECODE を、スカラーの imul のままにした 1 回目の形 (_sb7.hpp の V = 0) に替える
#include "_sb6.hpp"
#include "_sb7.hpp"
namespace sb7 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb1::encode(q.x[0], q.x[1], out); break;
            case DECODE: decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb6::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb6::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb7
using sb7::run;
