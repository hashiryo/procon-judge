#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木の持ち方なので、載せる
// モノイド (2x2 行列の積) は 1 つに揃える。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/Matrix.hpp"

using Mint = ModInt<1000000007>;
using Mat = Matrix<Mint>;

inline const Mat IDENTITY = Mat::identity(2);

inline Mat to_mat(const array<i64, 4> &x) {
  Mat m(2, 2);
  m[0][0] = Mint(x[0]), m[0][1] = Mint(x[1]);
  m[1][0] = Mint(x[2]), m[1][1] = Mint(x[3]);
  return m;
}

inline array<i64, 4> from_mat(const Mat &m) {
  return {(i64)m[0][0].val(), (i64)m[0][1].val(), (i64)m[1][0].val(), (i64)m[1][1].val()};
}

// 根側を左にして掛ける。可換でないので、Link-Cut 木の側は反転したときの積も持つ。
struct MatProd {
  using T = Mat;
  static T ti() { return IDENTITY; }
  static T op(const T &l, const T &r) { return l * r; }
};
