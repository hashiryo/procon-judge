#pragma once
// NeoLibrary の DiscreteLogTable を呼ぶ。中身は 5 回目 (_dlf5.hpp) の dlf5 と同じ手順。答えの列は dlf5 と同じく reserve
// してから huge page を頼み、0 で埋めてから添字で書く。
#include "../common.hpp"
#include "neo/number_theory/DiscreteLogTable.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif

inline vector<u32> run(u32 p, u32 g, const vector<u32>& qs) {
    const DiscreteLogTable D(p, g);
    vector<u32> ans;
    ans.reserve(qs.size());
#ifdef __linux__
    const uintptr_t b = reinterpret_cast<uintptr_t>(ans.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(ans.data() + qs.size()) + 4095) & ~uintptr_t(4095);
    if (b < e) madvise(reinterpret_cast<void*>(b), e - b, MADV_HUGEPAGE);
#endif
    ans.resize(qs.size());
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = (u32)D(qs[i]);
    return ans;
}
