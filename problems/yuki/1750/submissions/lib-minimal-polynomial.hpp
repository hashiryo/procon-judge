#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/MinimalPolynomial.hpp"

// 密行列とベクトルの列 v, Av, A^2 v, ... の最小多項式を Berlekamp-Massey で求め、
// x^T をその多項式で割った余りからベクトルの T 乗を組む。行列積は N^2 で済む。
struct Solver {
  int n;
  i64 days;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int n, i64 days, const vector<array<int, 2>> &edges) : n(n), days(days), edges(edges) {}

  void run() {
    using Mint = ModInt<998244353>;
    Matrix<Mint> mat(n, n);
    for (auto &e : edges) mat[e[0]][e[1]] = mat[e[1]][e[0]] = 1;
    Vector<Mint> vec(n);
    vec[0] = 1;
    ans = MinimalPolynomial(mat, vec).pow(days)[0].val();
  }

  i64 answer() const { return ans; }
};
