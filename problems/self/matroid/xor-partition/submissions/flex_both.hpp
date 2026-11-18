#pragma once
// 案 C の試作 (_shared/matroid/flex.hpp)。マトロイドの型は can_exchange と circuit の両方を持つ。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/flex.hpp"

struct Solver {
  const vector<u64> &vec;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : vec(vec), color(color), cap(cap) {}

  void run() {
    mtr::Xor64 g0(vec);
    mtr::Partition p0(color, cap);
    auto &g = g0;
    auto &p = p0;
    ans = flex::matroid_intersection(g, p);
  }

  vector<int> answer() const { return ans; }
};
