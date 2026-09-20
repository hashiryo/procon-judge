#pragma once
#include "common.hpp"
#include "mylib/algebra/Matrix.hpp"

// 遷移行列をそのまま繰り返し二乗する。1 回の掛け算が O(W^3) なので、
// W が大きいと効く。
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
      b = a.pow(obs[i][0] - y) * b;
      // 同じ行の障害物をまとめて落とす。
      int j = i;
      while (j < n && obs[i][0] == obs[j][0]) b[obs[j++][1]] = 0;
      i = j - 1;
      y = obs[i][0];
    }
    b = a.pow(h - 1 - y) * b;
    ans = b[w - 1];
  }

  i64 answer() const { return ans.val(); }
};
