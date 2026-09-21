#pragma once
// Library の characteristic_polynomial の定数項で det を出す (任意 mod なので除算を避ける)。
#include "../common.hpp"
#include "mylib/algebra/ModInt_Runtime.hpp"
#include "mylib/algebra/characteristic_polynomial.hpp"
struct Det {
 static u32 run(int N, u32 mod, const vector<vector<u32>>& a) {
  using Mint= ModInt_Runtime<int>;
  Mint::set_mod(mod);
  Matrix<Mint> A(N, N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) A[i][j]= Mint(a[i][j]);
  return characteristic_polynomial(A * -1, true)[0].val();
 }
};
