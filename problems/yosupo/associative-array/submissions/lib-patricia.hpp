#pragma once
// Library の SegmentTree_Patricia (mylib/data_structure/SegmentTree_Patricia.hpp) を、キーの 60 bit の 2 分木を
// 道の途中で縮めたものとして使う。作用素を持たない形で、値をそのまま葉に置く。キーは 2^60 未満 (10^18 以下) に限る。
#include "mylib/data_structure/SegmentTree_Patricia.hpp"
struct Solver {
 SegmentTree_Patricia<u64, false, 60> seg;
 void set(u64 k, u64 v) { seg.set(k, v); }
 u64 get(u64 k) { return seg.get(k); }
};
