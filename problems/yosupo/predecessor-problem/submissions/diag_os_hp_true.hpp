#pragma once
// 診断用。NeoLibrary の OrderedSet<true> の葉と節点の配列のうち 2 MB 以上のものを、2 MB 境界の mmap に置いて
// MADV_HUGEPAGE を頼む (_pages.hpp)。
#define PAGES_HP
#include "_pages.hpp"
#include "_neo_os.hpp"
using Solver = NeoOrderedSetSolver<true>;
