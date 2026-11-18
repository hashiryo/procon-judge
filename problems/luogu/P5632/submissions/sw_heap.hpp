#pragma once
// Stoer-Wagner の方法を、隣接の列と 2 分ヒープで回す O(nm log n) の形 (_shared/gmincut/sw_heap.hpp)。
#include "_shared/gmincut/sw_heap.hpp"
#define GMC_CORE gmc::sw_heap
#include "_shared/gmincut/solver.hpp"
