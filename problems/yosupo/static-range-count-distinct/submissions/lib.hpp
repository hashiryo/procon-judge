#pragma once
// 今の Library の RangeCountDistinct (mylib/data_structure/RangeCountDistinct.hpp)。
#include "pj.hpp"
#include "mylib/data_structure/RangeCountDistinct.hpp"
struct Solver {
 RangeCountDistinct rcd;
 explicit Solver(const vector<int>& a): rcd(a) {}
 int count_distinct(int l, int r) { return rcd.count(l, r); }
};
