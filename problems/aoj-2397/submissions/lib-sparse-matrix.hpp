#pragma once
#include "common.hpp"
#include "mylib/algebra/MinimalPolynomial.hpp"

// 行列を作らず、掛け算を関数として渡す。遷移は 3 重対角なので 1 回が O(W)
// で済み、最小多項式を作る費用も O(W^2) から落ちる。
struct Solver {
  int w;
  i64 h;
  vector<array<i64, 2>> obs;
  Mint ans;

  Solver(int w, i64 h, const vector<array<i64, 2>> &obs) : w(w), h(h), obs(obs) {}

  void run() {
    auto f = [&](const Vector<Mint> &v) {
      Vector<Mint> r(w);
      for (int i = 0; i < w; ++i) {
        r[i] += v[i];
        if (i) r[i] += v[i - 1];
        if (i + 1 < w) r[i] += v[i + 1];
      }
      return r;
    };
    Vector<Mint> b(w);
    b[0] = 1;
    i64 y = 0;
    const int n = (int)obs.size();
    for (int i = 0; i < n; ++i) {
      b = MinimalPolynomial(f, b).pow(obs[i][0] - y);
      int j = i;
      while (j < n && obs[i][0] == obs[j][0]) b[obs[j++][1]] = 0;
      i = j - 1;
      y = obs[i][0];
    }
    b = MinimalPolynomial(f, b).pow(h - 1 - y);
    ans = b[w - 1];
  }

  i64 answer() const { return ans.val(); }
};
