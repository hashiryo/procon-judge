#pragma once
// ライブラリを使う提出が共有するもの。客の希望色と在庫の色をまとめて座標圧縮し、
// 色ごとに客と在庫の数を数えるところはどちらの形式でも同じなので、ここに置く。
#include <algorithm>
#include "pj.hpp"
#include "mylib/misc/compress.hpp"

struct Counts {
  vector<i64> xs;    // 出てくる色 (昇順、重複なし)
  vector<i64> a, b;  // xs[i] を希望する客の数、xs[i] の在庫の種類数 (0 か 1)
};

inline Counts count_colors(const vector<i64> &a, const vector<i64> &b) {
  Counts c;
  c.xs = a;
  c.xs.insert(c.xs.end(), b.begin(), b.end());
  auto id = compress(c.xs);
  c.a.assign(c.xs.size(), 0), c.b.assign(c.xs.size(), 0);
  for (i64 v : a) ++c.a[id(v)];
  for (i64 v : b) ++c.b[id(v)];
  return c;
}
