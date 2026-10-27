#pragma once
// NeoLibrary の neo/sort/sort.hpp の sort を呼ぶ。中身は msd16_net32_ai と同じ手順で、作業用の配列は 2^19 byte 未満なら huge page を頼まない。
#include <vector>
#include "neo/sort/sort.hpp"
inline void run(std::vector<long long>& a) { sort(a); }
