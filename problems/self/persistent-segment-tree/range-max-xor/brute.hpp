#pragma once
// 愚直解 (pj testdata crosscheck 用)。区間を走査する。n, q <= 40 の小さい入力だけを扱う。
#include <algorithm>
#include <vector>

struct Solver {
  std::vector<int> a;
  explicit Solver(const std::vector<int> &a) : a(a) {}
  int max_xor(int l, int r, int x) const {
    int best = 0;
    for (int i = l; i < r; ++i) best = std::max(best, a[i] ^ x);
    return best;
  }
};
