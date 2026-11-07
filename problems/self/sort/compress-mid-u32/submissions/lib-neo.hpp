#pragma once
// NeoLibrary の neo/sort/compress.hpp の compress を呼ぶ。中身は pack_msd16_net32_fused_all と同じ手順。
#include <vector>
#include "neo/sort/compress.hpp"
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress(a); }
