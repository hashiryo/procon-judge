#pragma once
// Library の DisjointSparseTable (mylib/data_structure/DisjointSparseTable.hpp)。長さを 2 の冪に切り上げ、段 h では
// 幅 2^(h+1) の塊ごとに、真ん中から左へ向かう最小と右へ向かう最小を持つ。クエリは l と r - 1 の最上位の異なる bit
// の段で 2 つを引く。演算は std::function を通る。
#include "pj.hpp"
#include "mylib/data_structure/DisjointSparseTable.hpp"
struct Solver {
 DisjointSparseTable<u32> st;
 explicit Solver(const vector<u32>& a): st(a, [](u32 x, u32 y) { return x < y ? x : y; }) {}
 u32 query(int l, int r) { return st.prod(l, r); }
};
