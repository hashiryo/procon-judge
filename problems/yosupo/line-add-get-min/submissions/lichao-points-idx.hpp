#pragma once
#include "pj.hpp"
#include "_shared/lines/lichao.hpp"

// lichao-points と同じ木で、問いの葉の添字を二分探索で引かない形。呼ぶ側が座標を詰めて点の添字を持つ使い方を真似て、
// コンストラクタで (x, 問いの番号) を並べて順位を振り、問いでは順位を順に使う。
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
    for (auto &e : lines) t.add_line(e[0], e[1]);
  }

  void insert(i64 a, i64 b) { t.add_line(a, b); }

  i64 query(i64) { return t.query_index(idx[qi++]); }
};
