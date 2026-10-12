#pragma once
// 同じ値の前の位置を、添字を値で std::stable_sort して求める素直な形 (同じ値の添字は前から並ぶので、並びの 1 つ前が前の位置)。
// DP は _nos_dp.hpp。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <algorithm>
#include <numeric>
#include <vector>
#include "pj.hpp"
#include "_nos_dp.hpp"

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() {
    const int n = int(a.size());
    vector<int> idx(n), prev(n, -1);
    iota(idx.begin(), idx.end(), 0);
    stable_sort(idx.begin(), idx.end(), [&](int x, int y) { return a[x] < a[y]; });
    for (int k = 1; k < n; ++k)
      if (a[idx[k]] == a[idx[k - 1]]) prev[idx[k]] = idx[k - 1];
    ans = nos_dp::count(prev);
  }

  unsigned answer() const { return ans; }
};
