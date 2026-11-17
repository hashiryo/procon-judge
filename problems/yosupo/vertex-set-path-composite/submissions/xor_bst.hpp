#pragma once
#include "_bst.hpp"

// Library Checker の首位 (chaihf の 270199) と同じ、HLD の鎖ごとに位置の bit で形を決めた二分木を組む形 (中身は
// _bst.hpp)。木は隣の番号の xor で葉から剥がして作り、位置は軽い子を先に配る。1 回の問い合わせと更新が O(log N)。
struct Solver {
  comp_bst::Tree t;
  Solver(int n, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges) : t(n, f, edges) {}
  void set(int p, i64 c, i64 d) { t.set(p, c, d); }
  i64 composite(int u, int v, i64 x) { return t.composite(u, v, x); }
};
