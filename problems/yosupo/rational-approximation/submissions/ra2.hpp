#pragma once
// 2 回目 (_ra2.hpp): 1 回目の互除法で r > b/2 の段を次の商 1 の段とまとめる (動く端と止まった端で持ち、選びは分岐なし)
#include "_ra2.hpp"
inline void run(const std::vector<Query>& qs, u32* out) { ra2::run<0>(qs, out); }
