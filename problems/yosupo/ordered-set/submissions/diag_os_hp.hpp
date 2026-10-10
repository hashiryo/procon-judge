#pragma once
// 診断用。lib-neo (NeoLibrary の OrderedSet<true>) の葉と節点の配列のうち 2 MB 以上のものを、2 MB 境界の mmap に置いて
// MADV_HUGEPAGE を頼む (_shared/pages/aligned_new.hpp)。値の候補が 10^6 個のケースで、葉の配列は 3 MB から 7 MB ほど。
#define PAGES_HP
#include "_shared/pages/aligned_new.hpp"
#include "lib-neo.hpp"
