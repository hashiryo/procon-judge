#pragma once
// Library の linear_map_det (mylib/algebra/MinimalPolynomial.hpp) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/MinimalPolynomial.hpp"
inline u32 run(int N, int K, const vector<int>& a, const vector<int>& b, const vector<u32>& c) {
 using Mint= ModInt<998244353>;
 using Vec= Vector<Mint>;
 auto f= [&](const Vec& v) {
  Vec ret(N);
  for(int i= 0; i < K; i++) ret[a[i]]+= v[b[i]] * c[i];
  return ret;
 };
 return linear_map_det<Mint>(f, N).val();
}
