#pragma once
// ライブラリを使う提出が共有するもの。最小全域木を作るところは同じなので
// ここに置く。比べたいのは「木の上の区間へ chmin を入れる」持ち方。
#include <algorithm>
#include <numeric>
#include "pj.hpp"
#include "mylib/data_structure/UnionFind.hpp"

// 使う値の上限。これが残っていれば、その辺を使う木は作れない。
static constexpr int NONE = 1 << 30;

// 最小全域木に入る辺の番号を、軽い順に返す。総重量も返す。
struct Mst {
  vector<int> used;  // 辺ごとに 0 か 1
  i64 cost = 0;
};

inline Mst build_mst(int n, const vector<array<i64, 3>> &edges) {
  const int m = (int)edges.size();
  vector<int> ord(m);
  std::iota(ord.begin(), ord.end(), 0);
  std::sort(ord.begin(), ord.end(),
            [&](int l, int r) { return edges[l][2] < edges[r][2]; });
  UnionFind uf(n);
  Mst mst;
  mst.used.assign(m, 0);
  for (int i : ord)
    if (uf.unite((int)edges[i][0], (int)edges[i][1]))
      mst.cost += edges[i][2], mst.used[i] = 1;
  return mst;
}
