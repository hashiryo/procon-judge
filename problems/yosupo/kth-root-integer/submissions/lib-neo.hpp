#pragma once
// NeoLibrary の kth_root.hpp を呼ぶ。中身は 6 回目 (kr6.hpp) と同じ手順。
#include "../common.hpp"
#include "neo/number_theory/kth_root.hpp"

inline void run(const std::vector<Query>& qs, u64* out) {
    for (const auto& q : qs) *out++ = kth_root(q.a, q.k);
}
