#pragma once
// 診断用。lib-neo (NeoLibrary の OrderedSet<true>) の葉と節点の配列のうち 2 MB 以上のものを、MADV_NOHUGEPAGE で 4 KB の
// ページに留める (_shared/pages/aligned_new.hpp)。
#define PAGES_4K
#include "_shared/pages/aligned_new.hpp"
#include "lib-neo.hpp"
