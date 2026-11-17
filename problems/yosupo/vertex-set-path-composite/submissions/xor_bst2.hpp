#pragma once
#include "_bst.hpp"

// xor_bst と同じ鎖ごとの二分木 (_bst.hpp) で、問い合わせを composite_apply にしたもの。xor_bst は同じ鎖の区間と v の側で
// 1 次関数どうしを合成する (掛け算 3 回か 6 回) ので、一直線の木でセグメント木の 2 倍かかった。こちらはどの節も値に 1 回
// ずつ当て、v の側の節は積んでおいて最後に逆順で当てる。更新は xor_bst と同じ。
struct Solver {
  comp_bst::Tree t;
  Solver(int n, const vector<array<i64, 2>> &f, const vector<array<int, 2>> &edges) : t(n, f, edges) {}
  void set(int p, i64 c, i64 d) { t.set(p, c, d); }
  i64 composite(int u, int v, i64 x) { return t.composite_apply(u, v, x); }
};
