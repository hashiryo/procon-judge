#pragma once
// lib-neo と同じ形で、要素を範囲の判定を省いた MinPlus (_minplus_ab.hpp の MODE 0、この問題の重みは負にならないので答えは同じ)にしたもの。lib-neo の判定の重さを同じ回で比べる。測り終えたら消す。
#include "_char_mat.hpp"
#include "_minplus_ab.hpp"
#include "neo/linalg/SquareMatrix.hpp"
struct Solver {
 using R= MinPlusAB<0>;
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
