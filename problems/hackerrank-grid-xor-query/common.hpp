#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは静的な 2 次元の点集合に対する
// 長方形の XOR を取る構造なので、載せるモノイドと質問の回し方をここに置く。
#include "pj.hpp"

struct RangeXor {
  using T = int;
  static T ti() { return 0; }
  static T op(T l, T r) { return l ^ r; }
};

// Index は次を実装する。
//   Index(const vector<array<int, 3>> &pts);   // 点 (x, y) と値
//   int query(int a, int b, int c, int d);      // a <= x <= b, c <= y <= d の XOR
template <class Index> struct GridSolver {
  vector<array<int, 3>> pts;
  vector<array<int, 4>> qs;
  vector<i64> ans;

  GridSolver(const vector<array<int, 3>> &pts, const vector<array<int, 4>> &qs) : pts(pts), qs(qs) {}

  void run() {
    Index idx(pts);
    ans.assign(qs.size(), 0);
    for (size_t i = 0; i < qs.size(); ++i) ans[i] = idx.query(qs[i][0], qs[i][1], qs[i][2], qs[i][3]);
  }

  const vector<i64> &answer() const { return ans; }
};
