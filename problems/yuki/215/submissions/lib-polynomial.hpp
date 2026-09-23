#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/bostan_mori.hpp"
#include "mylib/fft/Polynomial.hpp"

// 多項式クラスで組み立てる。サイコロを 1 つ増やすごとに x^v を掛けて足す DP を
// 多項式の演算で書き、f(1) - f を x - 1 で割って「N 未満で止まらない」和を作る。
struct Solver {
  i64 n;
  int p, c;
  i64 ans = 0;

  Solver(i64 n, int p, int c) : n(n), p(p), c(c) {}

  void run() {
    using Mint = ModInt<1000000007>;
    using Poly = Polynomial<Mint, 1 << 17>;
    auto x = Poly::x();
    vector<Poly> a(p + 1), b(c + 1);
    a[0] = {1}, b[0] = {1};
    for (int v : {2, 3, 5, 7, 11, 13})
      for (int i = 1; i <= p; i++) a[i] += a[i - 1] * (x ^ v);
    for (int v : {4, 6, 8, 9, 10, 12})
      for (int i = 1; i <= c; i++) b[i] += b[i - 1] * (x ^ v);
    auto f = a[p] * b[c];
    ans = div_at<Mint, 1 << 17>((f(1) - f) / (x - 1), f - 1, n - 1).val();
  }

  i64 answer() const { return ans; }
};
