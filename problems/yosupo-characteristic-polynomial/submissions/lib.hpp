#pragma once
// Library の characteristic_polynomial (Hessenberg 化) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/characteristic_polynomial.hpp"
struct CharPoly {
 static vector<u32> run(int N, const vector<vector<u32>>& M) {
  using Mint= ModInt<998244353>;
  Matrix<Mint> a(N, N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < N; ++j) a[i][j]= Mint(M[i][j]);
  auto p= characteristic_polynomial(a);
  vector<u32> ret(N + 1);
  for(int i= 0; i <= N; ++i) ret[i]= p[i].val();
  return ret;
 }
};
