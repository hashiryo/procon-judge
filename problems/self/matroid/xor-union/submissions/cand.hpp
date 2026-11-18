#pragma once
// NeoLibrary に置く候補の口 mu::matroid_union (problems/_shared/matroid/union_algos.hpp) を使う。中身は union_bfs の
// Frozen、Add、Greedy (Fresh なし) で、型の操作は UnionAdapter を通して呼ぶ。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<mu::Xor> ms(k, mu::Xor(vec));
    ans = mu::matroid_union(ms);
  }

  vector<int> answer() const { return ans; }
};
