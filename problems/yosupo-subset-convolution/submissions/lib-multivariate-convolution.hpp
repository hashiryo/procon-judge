#pragma once
#include "common.hpp"
#include "mylib/fft/MultiVariateConvolution.hpp"

// 各変数の次数を 2 に制限した多変数畳み込みとして解く。部分集合畳み込みを
// 特別扱いしないぶん遅いが、同じ答えが出る。
struct Solver {
  int n;
  vector<Mint> a, b, c;

  Solver(int n, const vector<i64> &a, const vector<i64> &b)
      : n(n), a(to_mint(a)), b(to_mint(b)) {}

  void run() {
    MultiVariateConvolution mvc(vector(n, 2));
    c = mvc.convolve<Mint, 1 << 21, 20>(a, b);
  }

  vector<i64> answer() const { return from_mint(c); }
};
