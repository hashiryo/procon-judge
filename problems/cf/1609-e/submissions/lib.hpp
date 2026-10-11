#pragma once
// Library の SegmentTree に Matrix<Algebra<min-plus>> (大きさを実行時に決める行列) を載せる。全体の積は prod(0, n) で読む。
#include "../common.hpp"
#include "_char_mat.hpp"
#include "mylib/algebra/Algebra.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/data_structure/SegmentTree.hpp"
struct Solver {
 struct MP {
  using T= int;
  static constexpr int o= CM_INF;  // 和 (min) の単位元
  static constexpr int i= 0;       // 積 (+) の単位元
  static T add(T a, T b) { return min(a, b); }
  static T mul(T a, T b) { return min(a + b, CM_INF); }
 };
 using R= Algebra<MP>;
 using Mat= Matrix<R>;
 struct Mono {
  using T= Mat;
  static T ti() { return Mat::identity(3); }
  static T op(const T& l, const T& r) { return l * r; }
 };
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r(3, 3);
  for(int i= 0; i < 3; ++i)
   for(int j= 0; j < 3; ++j) r[i][j]= R(m[i * 3 + j]);
  return r;
 }
 int n;
 SegmentTree<Mono> seg;
 explicit Solver(const string& s): n(s.size()), seg(n, [&](int i) { return to_mat(s[i]); }) {}
 int update(int x, char c) {
  seg.set(x, to_mat(c));
  const Mat t= seg.prod(0, n);
  return min({t[0][0].x, t[0][1].x, t[0][2].x});
 }
};
