#pragma once
// 20 回目: ここまでで速かった手を問ごとに選ぶ。ENCODE は r > b/2 の段をまとめる分岐なしの互除法 (5 回目)、DECODE は
// vpmuludq (17 回目)、LCA は 2 本の普通の互除法 (1 回目)、ANCESTOR は普通の互除法と祖先が無いと分かった段での打ち切り
// (6 回目。段をまとめる 5 回目と速さが変わらず、手順が短い)、RANGE は逆元 1 つと割り切れる割り算 1 つ (9 回目)。
#include "_sb5.hpp"
#include "_sb6.hpp"
#include "_sb9.hpp"
#include "_sb17.hpp"
namespace sb20 {
inline u32* run(const Input& in, u32* out) {
    for (const auto& q : in.qs) {
        switch (q.type) {
            case ENCODE: out = sb5::encode(q.x[0], q.x[1], out); break;
            case DECODE: sb17::decode(in.path.data() + q.off, q.len, out), out += 2; break;
            case LCA: sb1::lca(q.x[0], q.x[1], q.x[2], q.x[3], out), out += 2; break;
            case ANCESTOR: sb6::ancestor(q.x[0], q.x[1], q.x[2], out), out += 2; break;
            default: sb9::range(q.x[0], q.x[1], out), out += 4; break;
        }
    }
    return out;
}
}  // namespace sb20
using sb20::run;
