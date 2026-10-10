#pragma once
// 診断用。NeoLibrary の OrderedSet<true> の葉と節点の配列のうち 2 MB 以上のものを、MADV_NOHUGEPAGE で 4 KB のページに
// 留める (_pages.hpp)。
#define PAGES_4K
#include "_pages.hpp"
#include "_neo_os.hpp"
using Solver = NeoOrderedSetSolver<true>;
