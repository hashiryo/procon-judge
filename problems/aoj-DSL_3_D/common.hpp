#pragma once
// ライブラリを使う提出が共有するもの。値は int に収まる。
#include "pj.hpp"

inline vector<int> to_int(const vector<i64> &a) {
  return vector<int>(a.begin(), a.end());
}

inline vector<i64> to_i64(const vector<int> &a) {
  return vector<i64>(a.begin(), a.end());
}
