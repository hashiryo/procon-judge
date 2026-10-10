#pragma once
// 2-SAT に合わせた核 (_occ.hpp)。リテラルの出現の CSR を 1 つだけ組み、出現の数をそのまま剥がしの入次数に使う。頂点ごとの配列は鍵の 4 byte だけ。入次数 0 の頂点を剥がしてから、残りを path-based の DFS で分ける。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。
#include "_occ.hpp"
using Solver= occ::Solver<false>;
