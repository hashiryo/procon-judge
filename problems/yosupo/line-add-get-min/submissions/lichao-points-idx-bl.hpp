#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// lichao-points-idx と同じく問いの葉の添字を前もって持ち、追加を分岐を使わずに葉まで下る形 (add_line_full) にする。
struct Solver {
  vector<int> idx;  // t より先に作る (メンバは宣言の順に初期化される)
  LiChaoPoints t;
  int qi = 0;

  static LiChaoPoints build(const vector<i64> &xs, vector<int> &idx) {
    vector<pair<i64, int>> v(xs.size());
    for (size_t i = 0; i < xs.size(); ++i) v[i] = {xs[i], (int)i};
    sort(v.begin(), v.end());
    vector<i64> u;
    u.reserve(v.size());
    for (auto &[x, i] : v) {
      if (u.empty() || u.back() != x) u.push_back(x);
      idx[i] = (int)u.size() - 1;
    }
    return LiChaoPoints(std::move(u), LiChaoPoints::SortedUnique{});
  }

  Solver(const vector<array<i64, 2>> &lines, const vector<i64> &xs) : idx(xs.size()), t(build(xs, idx)) {
    for (auto &e : lines) t.add_line_full(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line_full(a, b); }

  i64 query(i64) { return t.query_index(idx[qi++]); }
};
