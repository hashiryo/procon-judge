#pragma once
// NeoLibrary の SquareMatrix<MinPlus<int>, 3> (neo/linalg/SquareMatrix.hpp と neo/algebra/semiring.hpp) を、点更新のセグメント木に
// 載せる。minplus_fixed と同じ形で、半環は NeoLibrary の MinPlus (INF は int の最大の半分で、積は片方が INF なら INF のまま)。
#include "_char_mat.hpp"
#include "neo/algebra/semiring.hpp"
#include "neo/linalg/SquareMatrix.hpp"
struct Solver {
 using R= MinPlus<int>;
 using Mat= SquareMatrix<R, 3>;
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r;
  for(int i= 0; i < 9; ++i) r.a[i]= m[i] >= CM_INF ? R() : R(m[i]);
  return r;
 }
 int sz= 1;
 vector<Mat> t;
 explicit Solver(const string& s) {
  const int n= s.size();
  while(sz < n) sz<<= 1;
  t.assign(2 * sz, Mat::identity());
  for(int i= 0; i < n; ++i) t[sz + i]= to_mat(s[i]);
  for(int i= sz - 1; i >= 1; --i) t[i]= t[2 * i] * t[2 * i + 1];
 }
 int update(int x, char c) {
  int i= x + sz;
  t[i]= to_mat(c);
  for(i>>= 1; i; i>>= 1) t[i]= t[2 * i] * t[2 * i + 1];
  const Mat& a= t[1];
  return min(a[0][0].x, min(a[0][1].x, a[0][2].x));
 }
};
