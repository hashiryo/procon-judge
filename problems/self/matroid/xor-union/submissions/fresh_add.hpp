#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、Frozen、Fresh、Add を使い、ベクトルを番号の順に 1 本ずつ入れる (Greedy なし)。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<mu::Xor> ms(k, mu::Xor(vec));
    ans = mu::union_bfs<true, true, true, false>(ms);
  }

  vector<int> answer() const { return ans; }
};
