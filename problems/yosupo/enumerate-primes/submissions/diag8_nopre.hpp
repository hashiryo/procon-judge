#pragma once
// 診断用 (答えは合わない)。ep8_b64_s16_p1k から EPD_SKIP = 1 の段を抜く (_diag8.hpp)。
#define EPD_SKIP 1
#include "_diag8.hpp"

inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) { return epd8::run<8192, 2048, 1000, 163, 64>(N, A, B); }
