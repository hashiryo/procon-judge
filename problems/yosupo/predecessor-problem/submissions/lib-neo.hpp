#pragma once
// NeoLibrary の FastSet (neo/base/ds/set/FastSet.hpp)。この問題で書き比べた fs_td を写したもので、huge page は頼まない。
#include "pj.hpp"
#include "neo/base/ds/set/FastSet.hpp"
struct Solver {
 FastSet s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
