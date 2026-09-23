#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは Union-Find の側なので、
// 載せる群は 1 つに揃える。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/Algebra.hpp"

using Mint = ModInt<998244353>;
using Mat = Matrix<Mint>;

// 2 次の特殊線形群。行列式が 1 なので、逆行列は余因子を並べ替えるだけで出る。
struct SL2 {
  using T = Mat;
  static inline T o = Mat::identity(2);
  static T add(const T &a, const T &b) { return a * b; }
  static T neg(const T &a) {
    return Mat{{a[1][1], -a[0][1]}, {-a[1][0], a[0][0]}};
  }
};

using G = Algebra<SL2>;

inline Mat to_mat(const array<i64, 4> &x) {
  Mat m(2, 2);
  m[0][0] = Mint(x[0]), m[0][1] = Mint(x[1]);
  m[1][0] = Mint(x[2]), m[1][1] = Mint(x[3]);
  return m;
}

inline void from_mat(const Mat &m, array<i64, 4> &out) {
  out[0] = m[0][0].val(), out[1] = m[0][1].val();
  out[2] = m[1][0].val(), out[3] = m[1][1].val();
}
