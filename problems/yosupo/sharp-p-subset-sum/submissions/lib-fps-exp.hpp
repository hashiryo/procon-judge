#pragma once
#include "common.hpp"
#include "mylib/fft/fps_exp.hpp"

// log(1 + x^s) を直接展開して足し合わせてから exp を取る。
// log(1 + y) = y - y^2/2 + y^3/3 - ... を y = x^s に入れた形で、s ごとに
// T/s 項ずつ足すので、前処理は調和級数で O(T log T)。
struct Solver {
  int t;
  vector<i64> s;
  vector<Mint> b;

  Solver(int t, const vector<i64> &s) : t(t), s(s) {}

  void run() {
    vector<int> c(t + 1, 0);
    for (i64 v : s) ++c[(int)v];
    vector<Mint> a(t + 1);
    for (int v = 1; v <= t; ++v)
      if (c[v])
        for (int j = 1; (i64)j * v <= t; ++j) {
          Mint x = Mint(c[v]) / j;
          a[j * v] += (j & 1) ? x : -x;
        }
    b = exp(a);
  }

  vector<i64> answer() const {
    vector<i64> r(t);
    for (int i = 1; i <= t; ++i) r[i - 1] = b[i].val();
    return r;
  }
};
