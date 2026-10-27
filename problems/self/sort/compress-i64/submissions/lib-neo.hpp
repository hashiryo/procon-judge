#pragma once
// NeoLibrary の neo/sort/compress.hpp の compress を呼ぶ。中身は msd16_net32_fused_ph と同じ手順。
#include <vector>
#include "neo/sort/compress.hpp"
inline std::vector<long long> run(std::vector<long long>& a) { return compress(a); }
