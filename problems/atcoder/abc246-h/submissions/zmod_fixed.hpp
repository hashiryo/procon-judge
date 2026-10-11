#pragma once
// 大きさを型で決める SquareMatrix<ZMod, 3> (_matrix.hpp) を、点更新のセグメント木 (_seg.hpp) に載せる。積は ZMod の演算子で、
// 積と和のたびに還元する。
#include "_gen_mat.hpp"
#include "_matrix.hpp"
#include "_seg.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 using Mat= SquareMatrix<Z, 3>;
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r;
  for(int i= 0; i < 9; ++i) r.a[i]= Z(m[i]);
  return r;
 }
 Seg<Mat> seg;
 explicit Solver(const string& s): seg(s.size(), Mat::identity(), [&](int i) { return to_mat(s[i]); }) {}
 u32 update(int x, char c) {
  seg.set(x, to_mat(c));
  return (seg.all()(2, 0) + seg.all()(2, 1)).val();
 }
};
