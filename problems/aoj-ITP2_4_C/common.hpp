#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木なので、載せる値の型を
// 揃える。元の列は int に収まる。
#include "pj.hpp"

inline vector<int> to_int(const vector<i64> &a) {
  return vector<int>(a.begin(), a.end());
}

inline vector<i64> to_i64(const vector<int> &a) {
  return vector<i64>(a.begin(), a.end());
}
