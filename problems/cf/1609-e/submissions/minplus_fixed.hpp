#pragma once
// 大きさを型で決める SquareMatrix<MinPlus<int>, 3> (_matrix.hpp) を、点更新のセグメント木 (_seg.hpp) に載せる。
#include "_char_mat.hpp"
#include "_matrix.hpp"
#include "_seg.hpp"
struct Solver {
 using R= MinPlus<int>;
 using Mat= SquareMatrix<R, 3>;
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r;
  for(int i= 0; i < 9; ++i) r.a[i]= R(m[i] >= CM_INF ? R::INF : m[i]);
  return r;
 }
 Seg<Mat> seg;
 explicit Solver(const string& s): seg(s.size(), Mat::identity(), [&](int i) { return to_mat(s[i]); }) {}
 int update(int x, char c) {
  seg.set(x, to_mat(c));
  const Mat& t= seg.all();
  return min({t(0, 0).x, t(0, 1).x, t(0, 2).x});
 }
};
