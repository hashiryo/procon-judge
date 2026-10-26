#pragma once
// NeoLibrary の SquareMatrix<ZMod<998244353>, 3> (neo/linalg/SquareMatrix.hpp) を、点更新のセグメント木に載せる。積は成分ごとに
// 3 個の積を u64 にためて 1 回で還元する (zmod_fixed_acc と同じ形)。セグメント木は _seg.hpp と同じ形をこの中に書く。
#include "_gen_mat.hpp"
#include "neo/linalg/SquareMatrix.hpp"
struct Solver {
 using Z= ZMod<998244353>;
 using Mat= SquareMatrix<Z, 3>;
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r;
  for(int i= 0; i < 9; ++i) r.a[i]= Z::raw(m[i]);
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
 u32 update(int x, char c) {
  int i= x + sz;
  t[i]= to_mat(c);
  for(i>>= 1; i; i>>= 1) t[i]= t[2 * i] * t[2 * i + 1];
  return (t[1][2][0] + t[1][2][1]).val();
 }
};
