#pragma once
// 8 回目: 6 回目 (sb6.hpp) の DECODE を、2 連をまとめて鎖を半分にした形 (_sb7.hpp の V = 1) に替える
#include "_sb6.hpp"
#include "_sb7.hpp"
namespace sb8 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb1::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb7::decode<1>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb6::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb6::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb8
using sb8::run;
