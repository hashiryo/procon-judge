#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木なので、載せる半群と
// 値の型を揃える。値は int に収まる。
#include "pj.hpp"

struct RangeMin {
  using T = int;
  static T op(T l, T r) { return l < r ? l : r; }
};

inline vector<int> to_int(const vector<i64> &a) {
  return vector<int>(a.begin(), a.end());
}
