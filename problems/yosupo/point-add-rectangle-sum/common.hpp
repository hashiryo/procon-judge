#pragma once
// ライブラリを使う提出が共有するもの。どちらも座標から値への表を受け取って
// 組むので、表を作るところまで揃える。
#include <map>
#include "pj.hpp"

struct RangeSum {
  using T = i64;
  static T ti() { return 0; }
  static T op(T l, T r) { return l + r; }
};

// 重なった点は重みを足す。後から足される座標は、重み 0 で場所だけ取っておく。
inline map<array<int, 2>, i64> to_map(const vector<array<i64, 3>> &points,
                                      const vector<array<int, 2>> &spots) {
  map<array<int, 2>, i64> mp;
  for (auto &p : points) mp[{(int)p[0], (int)p[1]}] += p[2];
  for (auto &s : spots) mp[{s[0], s[1]}];
  return mp;
}
