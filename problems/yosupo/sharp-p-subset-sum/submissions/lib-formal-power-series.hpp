#pragma once
#include "common.hpp"
#include "mylib/fft/FormalPowerSeries.hpp"

// 「同じ大きさの部品を高々 1 個ずつ選ぶ」集合の数え上げなので、個数の列に
// PSET をかけると出る。遅延評価なので、係数を読み出すところまで計測区間に
// 入れる。
struct Solver {
  using FPS = FormalPowerSeries<Mint>;

  int t;
  vector<i64> s;
  vector<Mint> b;

  Solver(int t, const vector<i64> &s) : t(t), s(s) {}

  void run() {
    vector<Mint> c(t + 1, Mint());
    for (i64 v : s) c[(int)v] += 1;
    FPS r = PSET(FPS(c));
    b.resize(t + 1);
    for (int i = 0; i <= t; ++i) b[i] = r[i];
  }

  vector<i64> answer() const {
    vector<i64> r(t);
    for (int i = 1; i <= t; ++i) r[i - 1] = b[i].val();
    return r;
  }
};
