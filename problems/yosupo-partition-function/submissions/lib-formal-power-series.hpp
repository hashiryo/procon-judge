#pragma once
#include "common.hpp"
#include "mylib/fft/FormalPowerSeries.hpp"

// 分割数は「大きさ 1, 2, 3, ... の部品を何個でも選ぶ」多重集合の数え上げなので、
// 全項 1 の級数に MSET をかけると出る。遅延評価なので、係数を N+1 個読み出す
// ところまで計測区間に入れる。
struct Solver {
  using FPS = FormalPowerSeries<Mint>;

  int n;
  vector<Mint> b;

  explicit Solver(int n) : n(n) {}

  void run() {
    FPS r = MSET(FPS(vector<Mint>(n + 1, Mint(1))));
    b.resize(n + 1);
    for (int i = 0; i <= n; ++i) b[i] = r[i];
  }

  vector<i64> answer() const { return from_mint(b); }
};
