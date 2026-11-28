#pragma once
// 今の Library の MultiVariateConvolution (mylib/fft/MultiVariateConvolution.hpp、ランクの変数 chi) を問題の口で呼ぶ。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/MultiVariateConvolution.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 std::vector<int> dim;
 std::vector<Mint> f, g, h;

 Solver(const std::vector<int>& d, const std::vector<i64>& a, const std::vector<i64>& b): dim(d), f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Mint(a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Mint(b[i]);
 }

 void run() {
  MultiVariateConvolution mvc(dim);
  h= mvc.convolve(f, g);
 }

 std::vector<i64> answer() const {
  std::vector<i64> r(h.size());
  for (size_t i= 0; i < h.size(); ++i) r[i]= h[i].val();
  return r;
 }
};
