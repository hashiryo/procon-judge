#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/MinimalPolynomial.hpp"

// 行列を持たず、辺の一覧からベクトルを写す関数で最小多項式を求める。1 回の写像は
// 辺の数に比例するので、疎なグラフでは密行列より軽い。
struct Solver {
  int n;
  i64 days;
  vector<array<int, 2>> edges;
  i64 ans = 0;

  Solver(int n, i64 days, const vector<array<int, 2>> &edges) : n(n), days(days), edges(edges) {}

  void run() {
    using Mint = ModInt<998244353>;
    using Vec = Vector<Mint>;
    auto f = [&](const Vec &v) {
      Vec ret(n);
      for (auto &e : edges) ret[e[0]] += v[e[1]], ret[e[1]] += v[e[0]];
      return ret;
    };
    Vec vec(n);
    vec[0] = 1;
    ans = MinimalPolynomial(f, vec).pow(days)[0].val();
  }

  i64 answer() const { return ans; }
};
