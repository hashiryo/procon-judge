#pragma once
// 大きさを実行時に決める DynMatrix<ZMod> (_matrix.hpp、vector に持つ) を、zmod_fixed と同じセグメント木に載せる。
// 大きさを型で決めない場合の代価 (積のたびの確保と、展開されないループ) を見る。
#include "_gen_mat.hpp"
#include "_matrix.hpp"
#include "_seg.hpp"
struct Solver {
 using Z= ZMod<MOD>;
 using Mat= DynMatrix<Z>;
 static Mat to_mat(char c) {
  const auto& m= char_mat(c);
  Mat r(3);
  for(int i= 0; i < 9; ++i) r.a[i]= Z(m[i]);
  return r;
 }
 Seg<Mat> seg;
 explicit Solver(const string& s): seg(s.size(), Mat::identity(3), [&](int i) { return to_mat(s[i]); }) {}
 u32 update(int x, char c) {
  seg.set(x, to_mat(c));
  return (seg.all()(2, 0) + seg.all()(2, 1)).val();
 }
};
