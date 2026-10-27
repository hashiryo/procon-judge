#pragma once
// NeoLibrary の neo/sort/argsort.hpp の argsort を呼ぶ。Intel の CPU では pack_msd8x2_net32、ほかでは pack_msd16_net32_ai と同じ
// 手順で、返す列は pack_lsd16_ph と同じく huge page を頼んでから 0 で埋める。
#include <vector>
#include "neo/sort/argsort.hpp"
inline std::vector<unsigned> run(const std::vector<unsigned>& a) { return argsort<unsigned>(a); }
