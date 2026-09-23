#pragma once
#include "common.hpp"
#include "mylib/algebra/Matrix.hpp"
#include "mylib/algebra/MinimalPolynomial.hpp"

// 行列とベクトルの組に対する最小多項式を求めてから、多項式の剰余で冪を出す。
// 行列同士の掛け算が要らないので O(W^2 log k) に落ちる。
struct Solver {
  int w;
  i64 h;
  vector<array<i64, 2>> obs;
  Mint ans;

  Solver(int w, i64 h, const vector<array<i64, 2>> &obs) : w(w), h(h), obs(obs) {}

  void run() {
    Matrix<Mint> a(w, w);
    for (int i = 0; i < w; ++i) {
      a[i][i] = 1;
      if (i) a[i][i - 1] = 1;
      if (i + 1 < w) a[i][i + 1] = 1;
    }
    Vector<Mint> b(w);
    b[0] = 1;
    i64 y = 0;
    const int n = (int)obs.size();
    for (int i = 0; i < n; ++i) {
      b = MinimalPolynomial(a, b).pow(obs[i][0] - y);
      int j = i;
      while (j < n && obs[i][0] == obs[j][0]) b[obs[j++][1]] = 0;
      i = j - 1;
      y = obs[i][0];
    }
    b = MinimalPolynomial(a, b).pow(h - 1 - y);
    ans = b[w - 1];
  }

  i64 answer() const { return ans.val(); }
};
