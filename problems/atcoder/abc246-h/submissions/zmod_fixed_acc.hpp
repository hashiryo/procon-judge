#pragma once
// zmod_fixed の積を、3 個の積を u64 にためてから 1 回で還元する形 (SquareMatrixAcc<3>、_matrix.hpp) にしたもの。
// ZMod を要素にした行列の積を、ライブラリの中でこの形に切り替える案を比べる。
#include "_gen_mat.hpp"
#include "_matrix.hpp"
#include "_seg.hpp"
struct Solver {
 using Mat= SquareMatrixAcc<3>;
 static Mat to_mat(char c) {
  Mat r;
  r.a= char_mat(c);
  return r;
 }
 Seg<Mat> seg;
 explicit Solver(const string& s): seg(s.size(), Mat::identity(), [&](int i) { return to_mat(s[i]); }) {}
 u32 update(int x, char c) {
  seg.set(x, to_mat(c));
  const u32 v= seg.all()(2, 0) + seg.all()(2, 1);
  return v >= MOD ? v - MOD : v;
 }
};
