#pragma once
// k 組の直和と「同じベクトルは 1 本まで」の分割マトロイドの交差に直して、NeoLibrary の matroid_intersection (e1f86af) で
// 解く (problems/_shared/matroid/union_algos.hpp の union_isect)。台集合が k 倍になる。
#include "_shared/matroid/union_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  int k;
  vector<int> ans;

  Solver(const vector<u64> &vec, int k) : vec(vec), k(k) {}

  void run() {
    vector<mu::Xor> ms(k, mu::Xor(vec));
    ans = mu::union_isect(ms);
  }

  vector<int> answer() const { return ans; }
};
