#pragma once
// Library の SegmentTree に Matrix<ModInt> (大きさを実行時に決める行列) を載せる。全体の積は prod(0, n) で読む。
#include "../common.hpp"
#include "_gen_mat.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/data_structure/SegmentTree.hpp"
struct Solver {
 using Mint= ModInt<998244353>;
 using Mat= Matrix<Mint>;
 struct Mono {
  using T= Mat;
  static T ti() { return Mat::identity(3); }
  static T op(const T& l, const T& r) { return l * r; }
 };
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r(3, 3);
  for(int i= 0; i < 3; ++i)
   for(int j= 0; j < 3; ++j) r[i][j]= Mint(m[i * 3 + j]);
  return r;
 }
 int n;
 SegmentTree<Mono> seg;
 explicit Solver(const string& s): n(s.size()), seg(n, [&](int i) { return to_mat(s[i]); }) {}
 u32 update(int x, char c) {
  seg.set(x, to_mat(c));
  const Mat t= seg.prod(0, n);
  return (t[2][0] + t[2][1]).val();
 }
};
