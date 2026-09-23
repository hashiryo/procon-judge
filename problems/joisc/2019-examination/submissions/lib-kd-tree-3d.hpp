#pragma once
#include <map>
#include "common.hpp"
#include "mylib/data_structure/KDTree.hpp"

// 生徒を (S, T, S + T) の 3 次元の点として kd 木に静的に載せ、質問ごとに右上の
// 直方体の総和を取る。並べ替えも逐次の追加も要らないが、3 次元の領域問い合わせは
// O(N^(2/3))。
struct Solver {
  vector<array<int, 2>> st;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  Solver(const vector<array<int, 2>> &st, const vector<array<int, 3>> &qs) : st(st), qs(qs) {}

  void run() {
    map<array<int, 3>, int> mp;
    for (auto &e : st) mp[{e[0], e[1], e[0] + e[1]}] += 1;
    KDTree<int, 3, RangeCount> kdt(mp);
    ans.assign(qs.size(), 0);
    for (size_t i = 0; i < qs.size(); ++i)
      ans[i] = kdt.prod_cuboid(qs[i][0], INF, qs[i][1], INF, qs[i][2], INF);
  }

  const vector<i64> &answer() const { return ans; }
};
