#pragma once
// 今の Library の多項式の合成 (Polynomial::operator()) を、base の Solver の形にする。
#include <vector>
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/Polynomial.hpp"

struct Solver {
 using Mint= ModInt<998244353>;
 using Poly= Polynomial<Mint>;
 int n;
 Poly f, g;
 std::vector<i64> out;

 Solver(const std::vector<i64>& a, const std::vector<i64>& b): n(int(a.size())), f(a.size()), g(b.size()) {
  for (int i= 0; i < int(a.size()); ++i) f[i]= Mint(a[i]);
  for (int i= 0; i < int(b.size()); ++i) g[i]= Mint(b[i]);
 }

 void run() {
  Poly c= f(g);
  c.resize(n, Mint(0));
  out.resize(n);
  for (int i= 0; i < n; ++i) out[i]= c[i].val();
 }

 std::vector<i64> coeffs() const { return out; }
};
