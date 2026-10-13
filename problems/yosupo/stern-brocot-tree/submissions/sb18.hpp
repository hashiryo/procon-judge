#pragma once
// 18 回目: 17 回目 (sb17.hpp) の RANGE を、r > b/2 の段をまとめる分岐なしの互除法 (_sb18.hpp) にする
#include "_sb5.hpp"
#include "_sb17.hpp"
#include "_sb18.hpp"
namespace sb18 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb17::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb5::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb18
using sb18::run;
