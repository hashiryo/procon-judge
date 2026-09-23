#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/Matrix.hpp"

// 隣接行列を繰り返し 2 乗する。O(N^3 log T)。
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
    ans = mat.pow(days)[0][0].val();
  }

  i64 answer() const { return ans; }
};
