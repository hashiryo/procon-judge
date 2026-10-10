#pragma once
// NeoLibrary の OrderedSet<true> (neo/data_structure/OrderedSet.hpp)。この問題で書き比べた bptree_c8b を写したもの。
#include "pj.hpp"
#include "neo/data_structure/OrderedSet.hpp"
struct Solver {
 OrderedSet<true> s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k).value_or(-1); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x).value_or(-1); }
 int next(int x) const { return s.next(x).value_or(-1); }
};
