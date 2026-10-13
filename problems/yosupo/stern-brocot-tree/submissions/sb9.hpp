#pragma once
// 9 回目: 5 回目 (_sb5.hpp) の DECODE を 7 回目のスカラーの形に、RANGE を _sb9.hpp の逆元 1 つと割り切れる割り算 1 つの形に替える
#include "_sb5.hpp"
#include "_sb7.hpp"
#include "_sb9.hpp"
namespace sb9 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb7::decode<0>(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb9
using sb9::run;
