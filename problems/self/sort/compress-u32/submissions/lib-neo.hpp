#pragma once
// NeoLibrary の neo/base/core/util/compress.hpp の compress を呼ぶ。中身は pack_msd16_net32_fused_all と同じ手順。
#include <vector>
#include "neo/base/core/util/compress.hpp"
inline std::vector<unsigned> run(std::vector<unsigned>& a) { return compress(a); }
