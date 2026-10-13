#pragma once
// 19 回目: 17 回目 (sb17.hpp) の DECODE で R と L の 2 連をまとめる (_sb17.hpp の V = 1)
#include "_sb5.hpp"
#include "_sb9.hpp"
#include "_sb17.hpp"
namespace sb19 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb17::decode<1>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb9::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb19
using sb19::run;
