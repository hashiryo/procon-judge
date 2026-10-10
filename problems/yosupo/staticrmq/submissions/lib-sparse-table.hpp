#pragma once
// Library の SparseTable (mylib/data_structure/SparseTable.hpp)。段ごとに vector を持ち、段 k + 1 を段 k の 2 つの
// min で作る。クエリは長さの最上位の bit の幅の 2 区間を重ねて引く。raw のときの lib.cpp と同じ中身。
#include "pj.hpp"
#include "mylib/data_structure/SparseTable.hpp"
struct Solver {
 struct Min {
  u32 operator()(u32 x, u32 y) const { return x < y ? x : y; }
 };
 SparseTable<u32, Min> st;
 explicit Solver(const vector<u32>& a): st(a, Min{}) {}
 u32 query(int l, int r) const { return st.prod(l, r); }
};
