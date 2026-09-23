#pragma once
// ライブラリを使う提出が共有するもの。出てくる点を先に全部集めるところはどちらの
// 実装でも同じなので、ここに置く。比べたいのは 2 次元の点集合に対する 1 点更新と
// 長方形の最大を取る構造。
#include <algorithm>
#include <map>
#include "pj.hpp"

struct RangeMax {
  using T = i64;
  static T ti() { return 0; }
  static T op(T l, T r) { return std::max(l, r); }
};

// 最初からある三角形は面積を値に、あとから足す三角形は値 0 で場所だけ確保する。
inline map<array<int, 2>, i64> collect(const vector<array<i64, 3>> &tri,
                                      const vector<array<i64, 4>> &qs) {
  map<array<int, 2>, i64> mp;
  for (auto &t : tri) {
    i64 &v = mp[{(int)t[0], (int)t[1]}];
    v = std::max(v, t[2]);
  }
  for (auto &q : qs)
    if (q[0] == 1) mp[{(int)q[1], (int)q[2]}];
  return mp;
}

// Engine は次を実装する。
//   Engine(const map<array<int, 2>, i64> &points);  // 点と初期値
//   void mul(int l, int r, i64 s);                   // 点 (l, r) の値を max で更新
//   i64 max_in(int l, int r);                        // x, y とも [l, r] にある点の最大。無ければ 0
template <class Engine> struct TriangleSolver {
  vector<array<i64, 3>> tri;
  vector<array<i64, 4>> qs;
  vector<i64> ans;

  TriangleSolver(const vector<array<i64, 3>> &tri, const vector<array<i64, 4>> &qs)
      : tri(tri), qs(qs) {}

  void run() {
    auto mp = collect(tri, qs);
    Engine eng(mp);
    ans.clear();
    for (auto &q : qs) {
      if (q[0] == 1) eng.mul((int)q[1], (int)q[2], q[3]);
      else {
        i64 v = eng.max_in((int)q[1], (int)q[2]);
        ans.push_back(v ? v : -1);
      }
    }
  }

  const vector<i64> &answer() const { return ans; }
};
