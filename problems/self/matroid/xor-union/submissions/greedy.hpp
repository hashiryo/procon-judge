#pragma once
// problems/_shared/matroid/union_algos.hpp の union_bfs (Edmonds のマトロイドの分割) で、Frozen、Fresh、Add と、そのまま入るベクトルを先に全部入れる Greedy を使う。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<mu::Xor> ms(k, mu::Xor(vec));
    ans = mu::union_bfs<true, true, true, true>(ms);
  }

  vector<int> answer() const { return ans; }
};
