#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、greedy.hpp から Frozen だけを外したもの (探索で失敗した要素も以後たどる)。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<mu::Xor> ms(k, mu::Xor(vec));
    ans = mu::union_bfs<false, true, true, true>(ms);
  }

  vector<int> answer() const { return ans; }
};
