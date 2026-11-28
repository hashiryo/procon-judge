#pragma once
// 愚直解 (pj testdata crosscheck 用)。版ごとに列を丸ごと写す。n, q <= 40 の小さい入力だけを扱う。
#include <vector>

struct Solver {
  std::vector<std::vector<long long>> ver;
  explicit Solver(const std::vector<long long> &a, int) : ver{a} {}
  void set(int v, int i, long long x) {
    std::vector<long long> b = ver[v];
    b[i] = x;
    ver.push_back(std::move(b));
  }
  long long sum(int v, int l, int r) const {
    long long s = 0;
    for (int i = l; i < r; ++i) s += ver[v][i];
    return s;
  }
};
