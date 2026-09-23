#pragma once
#include "common.hpp"
#include "mylib/string/z_algorithm.hpp"

// Z アルゴリズム。A と B をつないだ列の Z 配列の B の部分が、そのまま A との最長
// 共通接頭辞になる (A の長さを超えたぶんは切る)。全体で O(N + M)。
struct Match {
  static vector<int> prefix_matches(const vector<int> &a, const vector<int> &b) {
    vector<int> s(a);
    s.insert(s.end(), b.begin(), b.end());
    vector<int> z = z_algorithm(s);
    vector<int> r(z.begin() + a.size(), z.end());
    for (int &x : r) x = std::min(x, (int)a.size());
    return r;
  }
};

using Solver = SequenceSolver<Match>;
