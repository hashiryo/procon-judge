#pragma once
// 17 回目: 9 回目 (sb9.hpp) の DECODE を vpmuludq の形 (_sb17.hpp) にする
#include "_sb5.hpp"
#include "_sb9.hpp"
#include "_sb17.hpp"
namespace sb17 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb9::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb17
using sb17::run;
