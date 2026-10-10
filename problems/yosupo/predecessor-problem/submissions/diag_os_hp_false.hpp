#pragma once
// 診断用。NeoLibrary の OrderedSet<false> の葉と節点の配列のうち 2 MB 以上のものを、2 MB 境界の mmap に置いて
// MADV_HUGEPAGE を頼む (_shared/pages/aligned_new.hpp)。
#define PAGES_HP
#include "_shared/pages/aligned_new.hpp"
#include "_neo_os.hpp"
using Solver = NeoOrderedSetSolver<false>;
