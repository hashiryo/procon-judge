#pragma once
#include <algorithm>
#include "pj.hpp"
#include "mylib/string/SuffixArray.hpp"

// 接尾辞配列を作って P を二分探索する。1 本の P を探すだけなら Z 配列より
// 重いが、作ったあとは別の P を O(|P| log |T|) で引ける。
struct Solver {
  string t, p;
  vector<int> pos;

  Solver(const string &t, const string &p) : t(t), p(p) {}

  void run() {
    SuffixArray sa(t);
    auto [l, r] = sa.pattern_matching(p);
    pos.assign(sa.begin() + l, sa.begin() + r);
    std::sort(pos.begin(), pos.end());
  }

  const vector<int> &answer() const { return pos; }
};
