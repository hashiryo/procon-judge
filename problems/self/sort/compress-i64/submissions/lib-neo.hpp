#pragma once
// NeoLibrary の neo/base/core/util/compress.hpp の compress を呼ぶ。中身は msd16_net32_fused_ph と同じ手順。
#include <vector>
#include "neo/base/core/util/compress.hpp"
inline std::vector<long long> run(std::vector<long long>& a) { return compress(a); }
