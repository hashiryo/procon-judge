#pragma once
// NeoLibrary の neo/sort/sort.hpp の sort を呼ぶ。64 個以下は net_avx2_ov のネットワーク、2^15 個以下か Intel の CPU では
// qsort_avx2_oop_all のクイックソート、それより長い列は lsd_adaptive_hp_cfu の LSD で並べる。
#include <vector>
#include "neo/sort/sort.hpp"
inline void run(std::vector<unsigned>& a) { sort(a); }
