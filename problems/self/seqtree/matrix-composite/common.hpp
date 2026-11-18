#pragma once
// 提出が共有するもの。値は 998244353 を法とする 2×2 行列で、積は可換でない。比べたいのは木なので、積の書き方はここで揃える。
#include "pj.hpp"

constexpr u32 MAT_MOD = 998244353;

struct Mat {
  u32 a, b, c, d;  // [[a, b], [c, d]]
};

inline Mat mat_mul(const Mat &x, const Mat &y) {
  return {u32((u64(x.a) * y.a + u64(x.b) * y.c) % MAT_MOD), u32((u64(x.a) * y.b + u64(x.b) * y.d) % MAT_MOD),
          u32((u64(x.c) * y.a + u64(x.d) * y.c) % MAT_MOD), u32((u64(x.c) * y.b + u64(x.d) * y.d) % MAT_MOD)};
}

// 行列の積のモノイド。可換でないので、反転には逆順の積も要る (commute は書かない)。
struct MatMul {
  using T = Mat;
  static T ti() { return {1, 0, 0, 1}; }
  static T op(const T &x, const T &y) { return mat_mul(x, y); }
};
