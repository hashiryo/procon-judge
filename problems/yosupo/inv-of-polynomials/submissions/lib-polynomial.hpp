#pragma once
// 今の Library の extgcd (mylib/fft/extgcd.hpp、half-gcd) を問題の口で呼ぶ。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/Polynomial.hpp"
#include "mylib/fft/extgcd.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 using Poly= Polynomial<Mint>;
 Poly f, g, x, y;
 int ddeg= -2;
 Mint d0= Mint(1);

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): f(a.size()), g(b.size()) {
  for (size_t i= 0; i < a.size(); ++i) f[i]= Mint(a[i]);
  for (size_t i= 0; i < b.size(); ++i) g[i]= Mint(b[i]);
 }

 void run() {
  Poly d= extgcd(f, g, x, y);
  ddeg= d.deg();
  if (ddeg == 0) d0= d[0];
 }

 bool impossible() const { return ddeg != 0; }

 std::vector<i64> answer() const {
  if (ddeg != 0 || x.deg() == -1) return {};
  const Mint id0= d0.inv();
  std::vector<i64> r(x.size());
  for (size_t i= 0; i < x.size(); ++i) r[i]= (x[i] * id0).val();
  return r;
 }
};
